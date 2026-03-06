#include "Model.h"
#include "Animation/SkinCluster.h"
#include "DirectXCommon.h"
#include "Function.h"
#include "Logger.h"
#include "ModelCommon.h"
#include "Object3d/Object3dCommon.h"
#include "SrvManager/SrvManager.h"
#include "TextureManager.h"
#include <cassert>
#include <fstream>
#include <sstream>

void Model::Initialize() {

	vertexResource_ = ModelCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);
	if (!modelData_.indices.empty()) {
		indexResource_ = ModelCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * modelData_.indices.size());
		indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
		indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData_.indices.size());
		indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

		uint32_t* indexData = nullptr;
		indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
		std::memcpy(indexData, modelData_.indices.data(), sizeof(uint32_t) * modelData_.indices.size());
		indexResource_->Unmap(0, nullptr);
	}
	// --- マテリアル用リソース ---
	// 3D用（球など陰影つけたいもの）
	// 必ず256バイト単位で切り上げる
	size_t alignedSize = (sizeof(Material) + 0xFF) & ~0xFF;
	materialResource_ = ModelCommon::GetInstance()->CreateBufferResource(alignedSize);
	mat3d = nullptr;
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	mat3d->enableLighting = true;
	mat3d->uvTransform = Function::MakeIdentity4x4();
	mat3d->shininess = 40.0f;
	mat3d->environmentCoefficient = 0.0f;

	materialResource_->Unmap(0, nullptr);

	TextureManager::GetInstance()->LoadTextureName(modelData_.material.textureFilePath);
	modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(modelData_.material.textureFilePath);
}
void Model::SetColor(Vector4 color) {

	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->color = color;
	materialResource_->Unmap(0, nullptr);
}
void Model::SetEnableLighting(bool enable) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->enableLighting = enable ? 1 : 0;
	materialResource_->Unmap(0, nullptr);
}
void Model::SetUvTransform(const Matrix4x4& uvTransform) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->uvTransform = uvTransform;
	materialResource_->Unmap(0, nullptr);
}
void Model::SetShininess(float shininess) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->shininess = shininess;
	materialResource_->Unmap(0, nullptr);
}
void Model::SetEnvironmentCoefficient(float coefficient) {
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&mat3d));
	mat3d->environmentCoefficient = coefficient;
	materialResource_->Unmap(0, nullptr);
}
void Model::Draw() { Draw(nullptr); }
void Model::Draw(const SkinCluster* skinCluster) { Draw(skinCluster, nullptr); }
void Model::Draw(const SkinCluster* skinCluster, const ID3D12Resource* materialResourceOverride) {
	// --- SRVヒープをバインド ---
	ID3D12DescriptorHeap* descriptorHeaps[] = {SrvManager::GetInstance()->GetDescriptorHeap().Get()};

	ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// --- VertexBufferViewを設定 ---
	if (skinCluster) {
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[] = {skinCluster->outputVertexBufferView};
		ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, _countof(vertexBufferViews), vertexBufferViews);
	} else {
		ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	}
	if (!modelData_.indices.empty()) {
		ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
	}

	// --- マテリアルCBufferの場所を設定 ---
    const D3D12_GPU_VIRTUAL_ADDRESS materialAddress =
        materialResourceOverride
            ? const_cast<ID3D12Resource*>(materialResourceOverride)->GetGPUVirtualAddress()
            : materialResource_.Get()->GetGPUVirtualAddress();
	ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialAddress);
	// --- SRVのDescriptorTableの先頭を設定 ---
	// TextureManagerからSRVのGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex);
	ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, srvHandle);

	// --- PointLight SRVのDescriptorTableを設定 ---
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(8, Object3dCommon::GetInstance()->GetPointLightSrvIndex());

	// --- SpotLight SRVのDescriptorTableを設定 ---
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(9, Object3dCommon::GetInstance()->GetSpotLightSrvIndex());

	// --- AreaLight SRVのDescriptorTableを設定 ---
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(10, Object3dCommon::GetInstance()->GetAreaLightSrvIndex());

	// --- Environment Map SRVのDescriptorTableを設定 ---
	SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(11, Object3dCommon::GetInstance()->GetEnvironmentMapSrvIndex());
	if (!Object3dCommon::GetInstance()->IsShadowMapPassActive()) {
		SrvManager::GetInstance()->SetGraphicsRootDescriptorTable(12, Object3dCommon::GetInstance()->GetShadowMapSrvIndex());
	}
	// --- 描画！（DrawCall）---
	if (!modelData_.indices.empty()) {
		ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(static_cast<UINT>(modelData_.indices.size()), 1, 0, 0, 0);
	} else {
		ModelCommon::GetInstance()->GetDxCommon()->GetCommandList()->DrawInstanced(
		    static_cast<UINT>(modelData_.vertices.size()), // 頂点数
		    1,                                             // インスタンス数
		    0,                                             // 開始頂点位置
		    0                                              // 開始インスタンス位置
		);
	}
}

// objfileを読む関数
void Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			// ここでx反転
			position.x *= -1.0f;

			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			// ここで法線もx反転

			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = {position, texcoord, normal};
			}
			// 回り順を逆にしてpush_back
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			std::string mtlFile;
			s >> mtlFile;
			modelData.material = LoadMaterialTemplateFile(directoryPath, mtlFile);
		}
	}

	modelData_ = modelData;
}
Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData matData;
	std::ifstream file(directoryPath + "/" + filename);
	std::string line;
	assert(file.is_open());
	while (std::getline(file, line)) {
		std::istringstream s(line);
		std::string identifier;
		s >> identifier;
		if (identifier == "map_Kd") {
			std::string textureFilePaths;
			s >> textureFilePaths;
			matData.textureFilePath = directoryPath + "/" + textureFilePaths;
		}
	}
	return matData;
}
void Model::LoadObjFileAssimp(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::string path = directoryPath + "/" + filename;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	assert(scene && scene->HasMeshes());

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex]; // OBJは1メッシュ想定
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		uint32_t vertexOffset = static_cast<uint32_t>(modelData.vertices.size());
		modelData.vertices.resize(vertexOffset + mesh->mNumVertices);
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			VertexData vertex;
			vertex.position = {position.x, position.y, position.z, 1.0f};
			vertex.normal = {normal.x, normal.y, normal.z};
			vertex.texcoord = {texcoord.x, texcoord.y};

			vertex.position.x *= -1.0f;
			vertex.normal.x *= -1.0f;

			modelData.vertices[vertexOffset + vertexIndex] = vertex;
		}

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				modelData.indices.push_back(vertexOffset + face.mIndices[element]);
			}
		}
	}

	// --- Material & Texture ---
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {

		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilepath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilepath);
			std::string texturePath = textureFilepath.C_Str();
			if (!texturePath.empty() && texturePath[0] == '*') {
				const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(textureFilepath.C_Str());
				if (embeddedTexture != nullptr) {
					if (embeddedTexture->mHeight == 0) {
						TextureManager::GetInstance()->LoadTextureFromMemory(texturePath, reinterpret_cast<const uint8_t*>(embeddedTexture->pcData), embeddedTexture->mWidth);
					} else {
						TextureManager::GetInstance()->LoadTextureFromRGBA8(texturePath, embeddedTexture->mWidth, embeddedTexture->mHeight, reinterpret_cast<const uint8_t*>(embeddedTexture->pcData));
					}
					modelData.material.textureFilePath = texturePath;
					modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(texturePath);
				}
			} else if (!texturePath.empty()) {
				modelData.material.textureFilePath = directoryPath + "/" + texturePath;
			}
		}
	}

	modelData.rootnode.localMatrix = Function::MakeIdentity4x4();
	modelData_ = std::move(modelData);
}
void Model::LoadObjFileGltf(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;
	std::string path = directoryPath + "/" + filename;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	if (!scene || !scene->HasMeshes()) {
		Logger::Log("LoadObjFileGltf failed: " + path + " " + importer.GetErrorString() + "\n");
		modelData_ = {};
		modelData_.rootnode.localMatrix = Function::MakeIdentity4x4();
		return;
	}

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex]; // OBJは1メッシュ想定
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		uint32_t vertexOffset = static_cast<uint32_t>(modelData.vertices.size());
		modelData.vertices.resize(vertexOffset + mesh->mNumVertices);
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			VertexData vertex;
			vertex.position = {position.x, position.y, position.z, 1.0f};
			vertex.normal = {normal.x, normal.y, normal.z};
			vertex.texcoord = {texcoord.x, texcoord.y};

			vertex.position.x *= -1.0f;
			vertex.normal.x *= -1.0f;

			modelData.vertices[vertexOffset + vertexIndex] = vertex;
		}

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				modelData.indices.push_back(vertexOffset + face.mIndices[element]);
			}
		}
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix;
			aiVector3D scale;
			aiVector3D translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = Function::MakeAffineMatrix({scale.x, scale.y, scale.z}, {rotate.x, -rotate.y, -rotate.z, rotate.w}, {-translate.x, translate.y, translate.z});
			jointWeightData.inverseBindPoseMatrix = bindPoseMatrix;

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				const aiVertexWeight& weight = bone->mWeights[weightIndex];
				jointWeightData.vertexWeights.push_back({weight.mWeight, weight.mVertexId + vertexOffset});
			}
		}
	}

	// --- Material & Texture ---
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {

		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilepath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilepath);
			std::string texturePath = textureFilepath.C_Str();
			if (!texturePath.empty() && texturePath[0] == '*') {
				const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(textureFilepath.C_Str());
				if (embeddedTexture != nullptr) {
					if (embeddedTexture->mHeight == 0) {
						TextureManager::GetInstance()->LoadTextureFromMemory(texturePath, reinterpret_cast<const uint8_t*>(embeddedTexture->pcData), embeddedTexture->mWidth);
					} else {
						TextureManager::GetInstance()->LoadTextureFromRGBA8(texturePath, embeddedTexture->mWidth, embeddedTexture->mHeight, reinterpret_cast<const uint8_t*>(embeddedTexture->pcData));
					}
					modelData.material.textureFilePath = texturePath;
					modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByfilePath(texturePath);
				}
			} else if (!texturePath.empty()) {
				modelData.material.textureFilePath = directoryPath + "/" + texturePath;
			}
		}
	}

	if (scene->mRootNode) {
		modelData.rootnode = NodeRead(scene->mRootNode);
	} else {
		modelData.rootnode.localMatrix = Function::MakeIdentity4x4();
	}
	modelData_ = std::move(modelData);
}
Model::Node Model::NodeRead(aiNode* node) {
	Node result;
	aiVector3D scale;
	aiVector3D translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);

	result.transform.scale = {scale.x, scale.y, scale.z};
	result.transform.quaternion = {rotate.x, -rotate.y, -rotate.z, rotate.w};
	result.transform.translate = {-translate.x, translate.y, translate.z};
	result.localMatrix = Function::MakeAffineMatrix(result.transform.scale, result.transform.quaternion, result.transform.translate);
	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = NodeRead(node->mChildren[childIndex]);
	}

	return result;
}