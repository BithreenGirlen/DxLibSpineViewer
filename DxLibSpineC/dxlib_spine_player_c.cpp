

#include "dxlib_spine_player_c.h"

CDxLibSpinePlayerC::CDxLibSpinePlayerC()
{

}

CDxLibSpinePlayerC::~CDxLibSpinePlayerC()
{

}

void CDxLibSpinePlayerC::Redraw()
{
	if (!m_drawables.empty())
	{
		SetTransformMatrix();

		if (!m_isDrawOrderReversed)
		{
			for (size_t i = 0; i < m_drawables.size(); ++i)
			{
				m_drawables[i]->Draw();
			}
		}
		else
		{
			for (long long i = m_drawables.size() - 1; i >= 0; --i)
			{
				m_drawables[i]->Draw();
			}
		}

		DxLib::ResetTransformTo2D();
	}
}

DxLib::FLOAT4 CDxLibSpinePlayerC::GetCurrentBounding() const
{
	if (m_drawables.empty())return {};

	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMaxWidth = FLT_MIN;
	float fMaxHeight = FLT_MIN;

	for (const auto& drawable : m_drawables)
	{
		const auto& rect = drawable->GetBoundingBox();

		fMinX = (std::min)(fMinX, rect.x);
		fMinY = (std::min)(fMinY, rect.y);
		fMaxWidth = (std::max)(fMaxWidth, rect.z);
		fMaxHeight = (std::max)(fMaxHeight, rect.w);
	}

	return { fMinX, fMinY, fMaxWidth, fMaxHeight };
}

/*標準尺度算出*/
void CDxLibSpinePlayerC::WorkOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	int iSkeletonWidth = static_cast<int>(m_fBaseSize.x);
	int iSkeletonHeight = static_cast<int>(m_fBaseSize.y);

	int iDisplayWidth = 0;
	int iDisplayHeight = 0;
#if defined _WIN32
	DxLib::GetDisplayMaxResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __ANDROID__
	DxLib::GetAndroidDisplayResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __APPLE__
	DxLib::GetDisplayResolution_iOS(&iDisplayWidth, &iDisplayHeight);
#endif
	if (iDisplayWidth == 0 || iDisplayHeight == 0)return;

	if (iSkeletonWidth > iDisplayWidth || iSkeletonHeight > iDisplayHeight)
	{
		float fScaleX = static_cast<float>(iDisplayWidth) / iSkeletonWidth;
		float fScaleY = static_cast<float>(iDisplayHeight) / iSkeletonHeight;

		if (fScaleX > fScaleY)
		{
			m_fDefaultScale = fScaleY;
		}
		else
		{
			m_fDefaultScale = fScaleX;
		}
	}
}

void CDxLibSpinePlayerC::WorkOutDefaultOffset()
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;

	for (const auto& pDrawable : m_drawables)
	{
		const auto& rect = pDrawable->GetBoundingBox();
		fMinX = (std::min)(fMinX, rect.x);
		fMinY = (std::min)(fMinY, rect.y);
	}

	m_fDefaultOffset = { fMinX == FLT_MAX ? 0 : fMinX, fMinY == FLT_MAX ? 0 : fMinY };
}

void CDxLibSpinePlayerC::SetTransformMatrix() const
{
	int iScreenWidth = 0;
	int iScreenHeight = 0;
	DxLib::GetDrawScreenSize(&iScreenWidth, &iScreenHeight);
	float fX = (m_fBaseSize.x * m_fSkeletonScale - iScreenWidth) / 2;
	float fY = (m_fBaseSize.y * m_fSkeletonScale - iScreenHeight) / 2;

	DxLib::MATRIX matrix = DxLib::MGetScale(DxLib::VGet(m_fSkeletonScale, m_fSkeletonScale, 1.f));
	DxLib::MATRIX tranlateMatrix = DxLib::MGetTranslate(DxLib::VGet(-fX, -fY, 0.f));
	matrix = DxLib::MMult(matrix, tranlateMatrix);

	DxLib::SetTransformTo2D(&matrix);
}
