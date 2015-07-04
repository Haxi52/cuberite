#pragma once

#include "Component.h"
#include "Vector3.h"

namespace Redstone
{

	class Lever : public Component
	{

	public:

		Lever(Vector3i location, BLOCKTYPE blockType, NIBBLETYPE meta) :
			Component(location, LEVER), m_Meta(meta), setOn(false), setOff(false)
		{
			LOGD("Lever created: %d %d %d", location.x, location.y, location.z);
			isOn = (meta & 0x08);
			attachedTo = location;
			switch (meta & 0x7)
			{
				case 0x0:
				case 0x7:
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
				case 0x6:
					attachedTo.Move({ 0, -1, 0 });
					break;
				default:
				{
					ASSERT(!"Unhandled block meta!");
				}
			}
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
			if (meta != m_Meta)
			{
				m_Meta = meta;
				bool state = (meta & 0x08);
				if (state)
				{
					setOn = true;
				}
				else
				{
					setOff = true;
				}
			}
		}
		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			return false;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			LOGD("Evaluating Lever (%d %d %d)", Location.x, Location.y, Location.z);

			if (setOn)
			{
				setOn = false;
				isOn = true;
				factory.PlaySound("random.click", Location, 0.6f);
				return GetAdjacent();
			}

			if (setOff)
			{
				setOff = false;
				isOn = false;
				factory.PlaySound("random.click", Location, 0.5f);
				return GetAdjacent();
			}

			return{};
		}

		~Lever()
		{
			LOGD("Lever destoryed (%d %d %d)", Location.x, Location.y, Location.z);
		}


	private:
		NIBBLETYPE m_Meta;
		bool isOn;
		bool setOn;
		bool setOff;
		Vector3i attachedTo;
	};

}
