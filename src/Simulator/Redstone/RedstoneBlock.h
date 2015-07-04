#pragma once

#include "Component.h"
#include "Vector3.h"

namespace Redstone
{

	class RedstoneBlock : public Component
	{

	public:

		RedstoneBlock(Vector3i location) :
			Component(location, REDSTONEBLOCK, true)
		{
			LOGD("RedstoneBlock created: %d %d %d", location.x, location.y, location.z);
		}

		virtual int CanStrongPower(Component * component)
		{
			return IsAdjacent(component->Location) ? 15 : 0;
		}

		virtual int CanWeakPower(Component * component)
		{
			return IsAdjacent(component->Location) ? 15 : 0;
		}

		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			return false;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			LOGD("Evaluating RedstoneBlock (%d %d %d)", Location.x, Location.y, Location.z);

			return{};
		}

		~RedstoneBlock()
		{
			LOGD("RedstoneBlock destoryed (%d %d %d)", Location.x, Location.y, Location.z);
		}

	};

}
