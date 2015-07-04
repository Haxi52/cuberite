
#include "Globals.h"
#include "ComponentFactory.h"

// Block types
#include "Torch.h"
#include "SolidBlock.h"
#include "Wire.h"
#include "Repeater.h"
#include "Button.h"
#include "Lever.h"
#include "RedstoneBlock.h"
#include "RedstoneLamp.h"

namespace Redstone
{

	ComponentFactory::ComponentFactory(cWorld & a_World, cRedstoneSimulatorChunkData * a_data) :
		m_World(a_World), m_Data(a_data)
	{ }

	void ComponentFactory::PlaySound(std::string name, Vector3i location, float pitch)
	{
		m_World.GetBroadcastManager().BroadcastSoundEffect(name, 
			static_cast<double>(location.x),
			static_cast<double>(location.y),
			static_cast<double>(location.z),
			0.5f, pitch);
	}


	ComponentPtr ComponentFactory::GetComponent(Vector3i location)
	{
		// first attempt to grab the component from chunk's cache
		// then create the component based on block type

		ComponentPtr component(m_Data->GetComponent(location));

		BLOCKTYPE blockType;
		NIBBLETYPE meta;
		
		// TODO: These are slow? would rather access chunk data directly.
		blockType = m_World.GetBlock(location);
		meta = m_World.GetBlockMeta(location);
		RedstoneType type = GetType(blockType);

		if (component == nullptr || component->Type != type)
		{
			component = ComponentPtr(CreateComponent(type, location, blockType, meta));
			if (component == nullptr)
			{
				m_Data->RemoveComponent(location);
				return nullptr;
			}
			m_Data->SetComponent(component);
		}
		else
		{
			// updating block meta allows repeater delay and button stats to change, without re-creating the instance.
			component->SetState(blockType, meta);
		}

		return component;
	}

	ComponentPtr ComponentFactory::CreateComponent(RedstoneType type, Vector3i location, BLOCKTYPE blockType, NIBBLETYPE meta)
	{
		switch (type)
		{
			case RedstoneType::TORCH:
				return ComponentPtr(std::make_shared<Torch>(location, blockType, meta));
			case RedstoneType::SOLIDBLOCK:
				return ComponentPtr(std::make_shared<SolidBlock>(location));
			case RedstoneType::WIRE:
				return ComponentPtr(std::make_shared<Wire>(location));
			case RedstoneType::REPEATER:
				return ComponentPtr(std::make_shared<Repeater>(location, blockType, meta));
			case RedstoneType::BUTTON:
				return ComponentPtr(std::make_shared<Button>(location, blockType, meta));
			case RedstoneType::LEVER:
				return ComponentPtr(std::make_shared<Lever>(location, blockType, meta));
			case RedstoneType::REDSTONEBLOCK:
				return ComponentPtr(std::make_shared<RedstoneBlock>(location));
			case RedstoneType::REDSTONELAMP:
				return ComponentPtr(std::make_shared<RedstoneLamp>(location, blockType, meta));
			default:
				return nullptr;
		}
	}
}






























