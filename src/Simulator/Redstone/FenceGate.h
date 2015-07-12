
#pragma once

#include "Component.h"
#include "Vector3.h"

namespace Redstone
{
	class FenceGate : public Component
	{

	public:

		FenceGate(Vector3i location, BLOCKTYPE blockType, NIBBLETYPE meta) :
			Component(location, RedstoneType::FENCEGATE), blockType(blockType), m_Meta(meta), pushUpdate(false)
		{
			LOGD("FenceGate created: %d %d %d", location.x, location.y, location.z);
			bool open = ((meta & 0x4) != 0);
			isOpen = open;
		}

		virtual bool CanConnect(Component * component)
		{
			return false;
		}

		virtual int CanStrongPower(Component * component)
		{
			return 0;
		}

		virtual int CanWeakPower(Component * component)
		{
			return 0;
		}

		virtual void SetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			bool open = ((meta & 0x4) != 0);
			m_Meta = meta;
			isOpen = open;
		}

		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			if (!pushUpdate)
			{
				return false;
			}
			pushUpdate = false;
			m_Meta = (isOpen) ? (m_Meta & (~0x4)) : (m_Meta | 0x4);
			block = blockType;
			meta = m_Meta;
			return true;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			LOGD("Evaluating FenceGate (%d %d %d)", Location.x, Location.y, Location.z);

			bool isPowered = false;
			for (auto side : GetAdjacent())
			{
				ComponentPtr comp = factory.GetComponent(side);
				if (comp == nullptr) continue;
				if (comp->CanWeakPower(this) > 0)
				{
					isPowered = true;
					break;
				}
			}
			
			if (isPowered != isOpen && !pushUpdate)
			{
				isOpen = isPowered;
				pushUpdate = true;
				if (isOpen)
				{
					factory.PlaySound("random.door_open", Location, 1.0f);
				}
				else
				{
					factory.PlaySound("random.door_close", Location, 1.0f);
				}

				return{ Location };
			}

			return{};
		}

		~FenceGate()
		{
			LOGD("FenceGate destoryed (%d %d %d)", Location.x, Location.y, Location.z);
		}

	private:
		BLOCKTYPE blockType;
		NIBBLETYPE m_Meta;
		bool pushUpdate;
		bool isOpen;
		int updateTick;
	};
}