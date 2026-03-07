#include "playerSwordTrail.h"
#include "Function.h"
#include <cmath>

namespace {
float Length(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

Vector3 NormalizeOrFallback(const Vector3& v, const Vector3& fallback) {
	if (Length(v) < 1e-4f) {
		return fallback;
	}
	return Function::Normalize(v);
}
} // namespace

void PlayerSwordTrail::Initialize() {
	points_.clear();
	if (!trail_) {
		trail_ = std::make_unique<Primitive>();
		trail_->Initialize(Primitive::Band, "Resources/3d/Circle.png");
		trail_->SetEnableLighting(false);
	}
}

void PlayerSwordTrail::Clear() { points_.clear(); }

void PlayerSwordTrail::Update(const Matrix4x4& swordWorldMatrix, bool isAttacking) {
	if (!isAttacking) {
		Clear();
		return;
	}

	Vector3 tipLocal = {0.0f, kTipLocalOffset, 0.0f};
	Vector3 tipWorld = Function::TransformVM(tipLocal, swordWorldMatrix);
	if (points_.empty()) {
		points_.push_back(tipWorld);
	} else {
		Vector3 delta = tipWorld - points_.back();
		if (Length(delta) >= kMinPointDistance) {
			points_.push_back(tipWorld);
		}
	}

	while (points_.size() > kMaxPoints) {
		points_.pop_front();
	}

	if (points_.size() < 2) {
		return;
	}

	UpdateTrailMesh();
}

void PlayerSwordTrail::Draw() {
	if (!trail_ || points_.size() < 2) {
		return;
	}
	trail_->Draw();
}

void PlayerSwordTrail::UpdateTrailMesh() {
	if (!trail_ || points_.size() < 2) {
		return;
	}
	const size_t pointCount = points_.size();
	std::vector<float> cumulativeLength(pointCount, 0.0f);
	float totalLength = 0.0f;
	for (size_t i = 1; i < pointCount; ++i) {
		const float segmentLength = Length(points_[i] - points_[i - 1]);
		totalLength += segmentLength;
		cumulativeLength[i] = totalLength;
	}
	if (totalLength < 1e-4f) {
		totalLength = 1.0f;
	}

	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	vertices.reserve(pointCount * 2);
	indices.reserve((pointCount - 1) * 6);

	const Vector3 cameraPos = camera_ ? camera_->GetWorldTranslate() : Vector3{0.0f, 0.0f, -1.0f};
	for (size_t i = 0; i < pointCount; ++i) {
		Vector3 direction{};
		if (i == 0) {
			direction = points_[1] - points_[0];
		} else if (i + 1 == pointCount) {
			direction = points_[i] - points_[i - 1];
		} else {
			direction = points_[i + 1] - points_[i - 1];
		}
		direction = NormalizeOrFallback(direction, {1.0f, 0.0f, 0.0f});
		Vector3 viewDir = NormalizeOrFallback(cameraPos - points_[i], {0.0f, 0.0f, -1.0f});
		Vector3 side = NormalizeOrFallback(Function::Cross(viewDir, direction), {0.0f, 1.0f, 0.0f});
		if (Length(side) < 1e-4f) {
			side = NormalizeOrFallback(Function::Cross(direction, {0.0f, 1.0f, 0.0f}), {0.0f, 0.0f, 1.0f});
		}
		Vector3 normal = NormalizeOrFallback(Function::Cross(direction, side), {0.0f, 0.0f, 1.0f});
		const float t = cumulativeLength[i] / totalLength;
		const float widthScale = 1.0f + (kHiltWidthScale - 1.0f) * t;
		const float halfWidth = kSegmentWidth * 0.5f * widthScale;
		Vector3 left = points_[i] - side * halfWidth;
		Vector3 right = points_[i] + side * halfWidth;
		vertices.push_back({
		    {left.x, left.y, left.z, 1.0f},
            {t, 1.0f},
            normal
        });
		vertices.push_back({
		    {right.x, right.y, right.z, 1.0f},
            {t, 0.0f},
            normal
        });
	}

	for (size_t i = 0; i + 1 < pointCount; ++i) {
		const uint32_t base = static_cast<uint32_t>(i * 2);
		indices.push_back(base);
		indices.push_back(base + 2);
		indices.push_back(base + 1);
		indices.push_back(base + 1);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
	}

	trail_->SetCamera(camera_);
	trail_->SetWorldMatrix(Function::MakeIdentity4x4());
	trail_->SetColor({0.2f, 0.7f, 1.0f, 0.6f});
	trail_->SetMeshData(vertices, indices);
	trail_->Update();
}