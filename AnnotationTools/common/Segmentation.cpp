#pragma once
#include "stdafx.h"
#include "Segmentation.h"

Segmentation::Segmentation(const std::vector<unsigned int>& ids, const std::string& sceneId /*= ""*/)
{
	clear();
	m_sceneName = sceneId;
	m_segmentIds = ids;
	for (unsigned int i = 0; i < m_segmentIds.size(); i++) {
		auto it = m_segIdsToVertIds.find(m_segmentIds[i]);
		if (it == m_segIdsToVertIds.end()) m_segIdsToVertIds[m_segmentIds[i]] = std::vector<unsigned int>(1, i);
		else it->second.push_back(i);
	}
}
