#include "BlendModeManager.h"
#include <cassert>

D3D12_BLEND_DESC BlendModeManager::SetBlendMode(BlendMode blendMode) {
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	auto& rtBlend = blendDesc.RenderTarget[0];
	rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	switch (blendMode) {
	case kBlendModeNone:
		rtBlend.BlendEnable = FALSE;
		break;

	case kBlendModeAlpha:
		rtBlend.BlendEnable = TRUE;
		rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
		rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
		rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
		rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case kBlendModeAdd:
		rtBlend.BlendEnable = TRUE;
		rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rtBlend.DestBlend = D3D12_BLEND_ONE;
		rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
		rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
		rtBlend.DestBlendAlpha = D3D12_BLEND_ONE;
		rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case kBlendModeSub:
		rtBlend.BlendEnable = TRUE;
		rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rtBlend.DestBlend = D3D12_BLEND_ONE;
		rtBlend.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT; // 減算
		rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
		rtBlend.DestBlendAlpha = D3D12_BLEND_ONE;
		rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case kBlendModeMultipy:
		rtBlend.BlendEnable = TRUE;
		rtBlend.SrcBlend = D3D12_BLEND_ZERO;
		rtBlend.DestBlend = D3D12_BLEND_SRC_COLOR; // 乗算
		rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
		rtBlend.SrcBlendAlpha = D3D12_BLEND_ZERO;
		rtBlend.DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	case kBlendScreen:
		rtBlend.BlendEnable = TRUE;
		rtBlend.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		rtBlend.DestBlend = D3D12_BLEND_ONE; // スクリーン
		rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
		rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
		rtBlend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		break;

	default:
		rtBlend.BlendEnable = FALSE;
		break;
	}

	return blendDesc;
}
