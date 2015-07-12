
#include "Globals.h"
#include "cRedstoneSimulatorChunkData.h"


void cRedstoneSimulatorChunkData::WakeUp(Vector3i location)
{
	if (m_ActiveBlocks.empty())
	{
		m_ActiveBlocks.push_back(location);
	}
	else
	{
		if (find(begin(m_ActiveBlocks), end(m_ActiveBlocks), location) == end(m_ActiveBlocks))
		{
			m_ActiveBlocks.push_back(location);
		}
	}
}

ComponentPtr cRedstoneSimulatorChunkData::GetComponent(Vector3i location)
{
	auto result = m_ChunkData.find(location);
	if (result == end(m_ChunkData))
	{
		return nullptr;
	}
	return result->second;
}

void cRedstoneSimulatorChunkData::RemoveComponent(Vector3i location)
{
	if (!m_ChunkData.erase(location))
	{
		return;
	}

	//// not sure if this is the best place, but update surounding blocks when one changes
	cVector3iList adjacents = {
		{ location.x + 1, location.y, location.z },
		{ location.x - 1, location.y, location.z },
		{ location.x, location.y + 1, location.z },
		{ location.x, location.y - 1, location.z },
		{ location.x, location.y, location.z + 1 },
		{ location.x, location.y, location.z - 1 },
	};

	for (Vector3i item : adjacents)
	{
		WakeUp(item);
	}
}

void cRedstoneSimulatorChunkData::SetComponent(ComponentPtr component)
{
	m_ChunkData[component->Location] = component;
}