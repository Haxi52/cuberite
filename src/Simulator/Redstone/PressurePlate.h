
#pragma once

#include "Component.h"
#include "Vector3.h"

namespace Redstone
{
	class PressurePlate : public Component
	{

	public:

		PressurePlate(Vector3i location, BLOCKTYPE blockType, NIBBLETYPE meta) :
			Component(location, RedstoneType::PRESSUREPLATE), m_Meta(meta), updateOnTick(-1), power(0), pushUpdate(false), blockType(blockType)
		{
			LOGD("PressurePlate created: %d %d %d", location.x, location.y, location.z);

			isOn = (meta & 0x01);
			detectItems = false;
			weightRatio = 0;
			switch (blockType)
			{
				case E_BLOCK_WOODEN_PRESSURE_PLATE:
					detectItems = true;
					break;
				case E_BLOCK_LIGHT_WEIGHTED_PRESSURE_PLATE:
					weightRatio = 1;
				case E_BLOCK_HEAVY_WEIGHTED_PRESSURE_PLATE:
					weightRatio = 10;
				case E_BLOCK_STONE_PRESSURE_PLATE:
					break;
			}

			attachedTo = location + Vector3i{ 0, -1, 0 };
		}

		virtual bool CanConnect(Component * component)
		{
			return Location.y == component->Location.y &&
				IsAdjacent(component->Location);
		}


		virtual int CanStrongPower(Component * component)
		{
			if (isOn && component->Location == attachedTo)
			{
				return power;
			}
			return 0;
		}

		virtual int CanWeakPower(Component * component)
		{
			if (!isOn || component->IsFullBlock)
			{
				return 0;
			}
			return IsAdjacent(component->Location) ? power : 0;
		}

		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			if (!pushUpdate)
			{
				return false;
			}
			pushUpdate = false;
			block = blockType;
			meta = m_Meta ^ (isOn ? 0x01 : 0x0);
			return true;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			//LOGD("Evaluating PressurePlate (%d %d %d)", Location.x, Location.y, Location.z);
			float lowerBound = Location.y;
			float upperBound = Location.y + 0.25f;
			int entityCount = 0;

			factory.ForEachEntity(Location, [&](cEntity * entity)
			{
				if (!detectItems && !(entity->IsPlayer() || entity->IsMob()))
				{
					return true;
				}

				float y = entity->GetPosY();
				if (y >= lowerBound && y <= upperBound)
				{
					float x = entity->GetPosX();
					float z = entity->GetPosZ();
					if (x >= Location.x && x < (Location.x + 1) &&
						z >= Location.z && z < (Location.z + 1))
					{
						++entityCount;
						if (weightRatio == 0)
						{
							return false;
						}
					}
				}
				return true;
			});
			int p = (weightRatio == 0) ? 15 : std::max(0, std::min(15, ((entityCount / weightRatio) + 1)));
			bool shouldBeOn = (entityCount > 0);
			bool update = false;

			if (!isOn && shouldBeOn)
			{
				factory.PlaySound("random.click", Location, 0.6f);
				isOn = true;
				pushUpdate = true;
				update = true;
				updateOnTick = ticks;
			}
			else if (isOn && !shouldBeOn)
			{
				if (updateOnTick >= ticks && !pushUpdate)
				{
					factory.PlaySound("random.click", Location, 0.5f);
					isOn = false;
					pushUpdate = true;
					update = true;
				}
				else
				{
					updateOnTick = ticks + 10;
					update = true;
				}
			}

			if (p != power || update)
			{
				power = p;
				return GetAdjacent(true);
			}


			// this causes pressure plates to be updated every tick
			// TODO: Find a better way to keep pressure plates updated.
			return { Location };
		}

		~PressurePlate()
		{
			LOGD("PressurePlate destoryed (%d %d %d) ", Location.x, Location.y, Location.z);
		}

	private:
		NIBBLETYPE m_Meta;
		BLOCKTYPE blockType;
		bool isOn;
		int updateOnTick;
		int power;
		bool pushUpdate;
		int weightRatio;
		bool detectItems;
		Vector3i attachedTo;
	};
}