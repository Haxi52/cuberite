
#pragma once

#include "Component.h"
#include "Vector3.h"

namespace Redstone
{
	class SolidBlock : public Component
	{

	public:

		SolidBlock(Vector3i location) :
			Component(location, RedstoneType::SOLIDBLOCK, true), isStrongPowered(false), power(0)
		{
			LOGD("SolidBlock created {%d %d %d}", location.x, location.y, location.z);
		}

		virtual int CanStrongPower(Component * component)
		{
			return isStrongPowered ? power : 0;
		}

		virtual int CanWeakPower(Component * component)
		{
			return power;
		}

		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			return false;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			UNUSED(ticks);
			LOGD("Evaluating SolidBlock (%d %d %d)", Location.x, Location.y, Location.z);
			// calculated value for power
			int cp = 0;
			// calculated value for strongly powered
			bool cSp = false;
			// create a list of components that may need updating
			cVector3iArray updateThese;
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
				if (comp->Type != RedstoneType::SOLIDBLOCK)
				{
					updateThese.push_back(side);
				}
			}

			if (cp != power || cSp != isStrongPowered)
			{
				power = cp;
				isStrongPowered = cSp;
				return updateThese;
			}

			return {};
		}

		~SolidBlock()
		{
			LOGD("SolidBlock destoryed (%d %d %d)", Location.x, Location.y, Location.z);
		}

	private:
		bool isStrongPowered;
		int power;
	};
}