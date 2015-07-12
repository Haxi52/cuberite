#pragma once

#include "Component.h"
#include "Vector3.h"

namespace Redstone
{

	class Button : public Component
	{

	public:

		Button(Vector3i location, BLOCKTYPE blockType, NIBBLETYPE meta) :
			Component(location, RedstoneType::BUTTON), m_Meta(meta), updateTick(-1), pushUpdate(false), setOn(false)
		{
			LOGD("Button created: %d %d %d", location.x, location.y, location.z);
			isWooden = (blockType == E_BLOCK_WOODEN_BUTTON);
			isOn = (meta & 0x08);
			attachedTo = location;
			switch (meta & 0x7)
			{
				case 0x0: 
					attachedTo.Move({ 0, 1, 0 });
					break;
				case 0x1: 
					attachedTo.Move({ -1, 0, 0 });
					break;
				case 0x2: 
					attachedTo.Move({ 1, 0, 0 });
					break;
				case 0x3: 
					attachedTo.Move({ 0, 0, -1 });
					break;
				case 0x4: 
					attachedTo.Move({ 0, 0, 1 });
					break;
				case 0x5:
					attachedTo.Move({ 0, -1, 0 });
					break;
				default:
				{
					ASSERT(!"Unhandled block meta!");
				}
			}
		}

		virtual bool CanConnect(Component * component)
		{
			return IsAdjacent(component->Location);
		}

		virtual int CanStrongPower(Component * component)
		{
			if (isOn && component->Location == attachedTo)
			{
				return 15;
			}
			return 0;
		}

		virtual int CanWeakPower(Component * component)
		{
			if (!isOn || component->IsFullBlock)
			{
				return 0;
			}
			return IsAdjacent(component->Location) ? 15 : 0;
		}
		virtual void SetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			if (!isOn && !pushUpdate && (meta & 0x08))
			{
				setOn = true;
				m_Meta = meta;
			}
		}
		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			if (!pushUpdate)
			{
				return false;
			}
			pushUpdate = false;
			block = isWooden ? E_BLOCK_WOODEN_BUTTON : E_BLOCK_STONE_BUTTON;
			meta = m_Meta;
			return true;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			LOGD("Evaluating button (%d %d %d)", Location.x, Location.y, Location.z);

			if (setOn)
			{
				setOn = false;
				isOn = true;
				updateTick = ticks + (isWooden ? 30 : 20);
				factory.PlaySound("random.click", Location, 0.6f);
				pushUpdate = true;
				return GetAdjacent(true);
			}

			if (updateTick > ticks){
				return{ Location };
			}

			if (isOn)
			{
				isOn = false;
				factory.PlaySound("random.click", Location, 0.5f);
				m_Meta &= 0x07;
				pushUpdate = true;
				return GetAdjacent(true);
			}

			return{};
		}

		~Button()
		{
			LOGD("Button destoryed (%d %d %d)", Location.x, Location.y, Location.z);
		}


	private:
		NIBBLETYPE m_Meta;
		bool isOn;
		bool setOn;
		int updateTick;
		bool pushUpdate;
		Vector3i attachedTo;
		bool isWooden;
	};

}
