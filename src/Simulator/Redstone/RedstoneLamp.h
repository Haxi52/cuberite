#pragma once

#include "Component.h"
#include "Vector3.h"

namespace Redstone
{

	class RedstoneLamp : public Component
	{

	public:

		RedstoneLamp(Vector3i location, BLOCKTYPE blockType, NIBBLETYPE meta) :
			Component(location, RedstoneType::REDSTONELAMP, true), m_Meta(meta), pushUpdate(false), isStrongPowered(false), power(0), updateTicks(0)
		{
			LOGD("RedstoneLamp created: %d %d %d", location.x, location.y, location.z);
			isOn = (blockType == E_BLOCK_REDSTONE_LAMP_ON);
		}

		virtual bool CanConnect(Component * component)
		{
			return false;
		}

		virtual int CanStrongPower(Component * component)
		{ 
			if (isStrongPowered && IsAdjacent(component->Location))
			{
				return power;
			}
			return 0;
		}

		virtual int CanWeakPower(Component * component)
		{
			if (IsAdjacent(component->Location))
			{
				return power;
			}
			return 0;
		}

		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			if (!pushUpdate)
			{
				return false;
			}
			pushUpdate = false;
			block = isOn ? E_BLOCK_REDSTONE_LAMP_ON : E_BLOCK_REDSTONE_LAMP_OFF;
			meta = m_Meta;
			return true;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			LOGD("Evaluating RedstoneLamp (%d %d %d)", Location.x, Location.y, Location.z);

			// calculated power
			int cp = 0;
			// calculated value for strongly powered
			bool cSp = false;
			bool isAdjacentToPower = false;

			for (auto side : GetAdjacent())
			{
				ComponentPtr comp = factory.GetComponent(side);
				if (comp == nullptr) continue;
				if (!comp->IsFullBlock)
				{
					int p = comp->CanStrongPower(this);
					if (p > 0)
					{
						cSp = true;
					}
					else
					{
						p = comp->CanWeakPower(this);
					}
					cp = std::max(cp, p);
				}
				else
				{
					isAdjacentToPower = (isAdjacentToPower || (comp->CanWeakPower(this) > 0));
				}
			}

			bool shouldBeOn = (isAdjacentToPower || cp > 0);
			bool updateSelf = false;
			if (!isOn && shouldBeOn)
			{
				isOn = true;
				pushUpdate = true;
				updateSelf = true;
			}
			else if (isOn && !shouldBeOn)
			{
				if (updateTicks >= ticks && !pushUpdate)
				{
					isOn = false;
					pushUpdate = true;
					updateSelf = true;
				}
				else
				{
					updateTicks = ticks + 8; // lamps turn off 4 ticks after loosing power
					updateSelf = true;
				}
			}
			
			if (cp != power || cSp != isStrongPowered)
			{
				power = cp;
				isStrongPowered = cSp;
				return GetAdjacent(updateSelf);
			}

			if (updateSelf)
			{
				return{ Location };
			}

			return{};
		}

		~RedstoneLamp()
		{
			LOGD("RedstoneLamp destoryed (%d %d %d)", Location.x, Location.y, Location.z);
		}

	private:
		NIBBLETYPE m_Meta;
		int updateTicks;
		bool pushUpdate;
		bool isOn;
		bool isStrongPowered;
		int power;
	};

}
