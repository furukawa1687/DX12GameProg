//----------------------------------------------------------------------------
//!	@file	shader.fx
//!	@brief	�V�F�[�_�[�T���v���R�[�h
//----------------------------------------------------------------------------

Texture2D		Texture[] : register(t0);
SamplerState	Sampler[] : register(s0);

// �萔�o�b�t�@
struct SceneConstantBuffer
{
	matrix	matWorld_;  // ���[���h�s��
	matrix	matView_;   // �r���[�s��
	matrix	matProj_;   // ���e�s��
};

ConstantBuffer<SceneConstantBuffer> scene : register(b0);


// ���_�V�F�[�_�[����
struct VS_INPUT
{
	float4	position_ : POSITION;
	float4	color_    : COLOR;
};

// �s�N�Z���V�F�[�_�[����
struct PS_INPUT
{
	float4	position_ : SV_POSITION;
	float4	color_    : COLOR;
	float2	uv_		  : TEXCOORD0;
};

//----------------------------------------------------------------------------
// ���_�V�F�[�_�[
//----------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	// ���W�ϊ�
	output.position_ = mul(scene.matWorld_, input.position_);
	output.position_ = mul(scene.matView_, output.position_);
	output.position_ = mul(scene.matProj_, output.position_);

	output.color_ = input.color_;

	// �ʒu����UV���v�Z
	{
		// 2D���e�ƃp�[�X�y�N�e�B�u�␳�𖳌��� (w=1.0)
		output.position_ /= output.position_.w;
		output.uv_ = output.position_.xy * float2(0.5, -0.5) + 0.5;
	}
	return output;
}

//----------------------------------------------------------------------------
// �e�N�X�`���Ȃ�
//----------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_TARGET
{
	return input.color_;
}

//----------------------------------------------------------------------------
// �e�N�X�`������
//----------------------------------------------------------------------------
float4 PS_Texture(PS_INPUT input) : SV_TARGET
{
	uint index = input.position_.x < 640 ? 0 : 1;
	return Texture[index].Sample(Sampler[0], input.uv_);// * input.color_;
}
