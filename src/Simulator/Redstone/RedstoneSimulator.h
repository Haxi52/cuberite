
#pragma once

#include "cRedstoneSimulatorChunkData.h"
#include "ComponentFactory.h"

#include "../Simulator.h"
#include "Chunk.h"
#include <unordered_map>
#include <vector>

// for now simulator and data are not in redstone namespace due to refactoring
using namespace Redstone;

class cRedstoneSimulator :
	public cSimulator
{
	
public:
	cRedstoneSimulator(cWorld & a_World) :
		cSimulator(a_World), m_ticks(0)
	{
		
	}
	
	virtual void Simulate(float a_dt);
	virtual void cRedstoneSimulator::SimulateChunk(std::chrono::milliseconds a_Dt, int a_ChunkX, int a_ChunkZ, cChunk * a_Chunk);
	virtual void WakeUp(int a_BlockX, int a_BlockY, int a_BlockZ, cChunk * a_Chunk);

	virtual cRedstoneSimulatorChunkData * CreateChunkData()
	{
		return new cRedstoneSimulatorChunkData;
	}

	virtual bool IsAllowedBlock(BLOCKTYPE a_BlockType) override
	{
		auto componentType = ComponentFactory::GetType(a_BlockType);
		return !(componentType == RedstoneType::UNKNOWN || componentType == RedstoneType::SOLIDBLOCK);
	}
	
protected:
	virtual void AddBlock(int a_BlockX, int a_BlockY, int a_BlockZ, cChunk * a_Chunk);

private:
	long m_ticks;
	
	// oh yea its crazy time
	// this is temporary, use chunk data per chunk, not globally like this.
	cRedstoneSimulatorChunkData data;

	inline cVector3iArray cRedstoneSimulator::GetAdjacent(Vector3i location);
} ;
