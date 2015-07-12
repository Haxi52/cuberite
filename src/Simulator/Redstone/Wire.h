#pragma once

#include "Component.h"
#include "Vector3.h"
#include <algorithm>

namespace Redstone
{
	class Wire : public Component
	{

	public:

		Wire(Vector3i location) :
			Component(location, RedstoneType::WIRE), power(0), lastUpdateTick(-1)
		{
			LOGD("Wire created {%d %d %d}", location.x, location.y, location.z);
		}

		virtual bool CanConnect(Component * component)
		{
			return true;
		}

		virtual int CanStrongPower(Component * component)
		{
			return 0;
		}

		virtual int CanWeakPower(Component * component)
		{
			if (component->Location == Down())
			{
				return power;
			}
			if (HasConnection(component->Location))
			{
				// we loose 1 power when traveling to another wire
				if (component->Type == RedstoneType::WIRE)
				{
					return std::max(power - 1, 0);
				}
				return power;
			}
			return 0;
		}

		virtual bool GetState(BLOCKTYPE & block, NIBBLETYPE & meta)
		{
			block = E_BLOCK_REDSTONE_WIRE;
			meta = static_cast<NIBBLETYPE>(power);
			return true;
		}

		virtual cVector3iArray Update(ComponentFactory & factory, int ticks)
		{
			// calculated power
			int cp = 0;
			// deturmines if the connections around us have changed (blocks placed/destroyed)
			bool connectionsUpdated = false;

			LOGD("Evaluating Wire (%d %d %d)", Location.x, Location.y, Location.z);


			ComponentPtr down = factory.GetComponent(Down());
			ComponentPtr up = factory.GetComponent(Up());
			if (down != nullptr && down->IsFullBlock)
			{
				cp = std::max(cp, down->CanStrongPower(this));
			}
			if (up != nullptr)
			{
				if (up->IsFullBlock)
				{
					cp = std::max(cp, up->CanStrongPower(this));
				}
				else if (up->CanConnect(this))
				{
					cp = std::max(cp, up->CanWeakPower(this));
				}
			}

			if (lastUpdateTick != ticks)
			{
				std::vector<int> oldConnections;
				oldConnections.swap(Connections);
				lastUpdateTick = ticks;
				cp = std::max(cp, UpdateConnections(factory, down, up));

				connectionsUpdated = (oldConnections != Connections);
			}
			else
			{
				auto calculatePower = [&](Vector3i side) {
					auto comp = factory.GetComponent(side);
					if (comp != nullptr && comp->IsFullBlock)
					{
						cp = std::max(cp, comp->CanStrongPower(this));
					}
					else if (comp != nullptr)
					{
						cp = std::max(cp, comp->CanWeakPower(this));
					}
				};

				for (auto i : Connections)
				{
					calculatePower(ToSide(i));
				}
				// always grab power from the sides
				for (auto i : GetLateralIndices())
				{
					calculatePower(ToSide(i));
				}
			}

			// cause an update if the power level changes, or if the connections change.
			if (cp != power || connectionsUpdated)
			{
				power = cp;
				auto sides = GetAdjacent(true);
				for (auto i : Connections)
				{
					// laterals are already included, this is just for those connetions 'far away'
					if (i < 4 || i > 7)
					{
						sides.push_back(ToSide(i));
					}
				}
				return sides;
			}

			return{};
		}

		~Wire()
		{
			LOGD("Wire destoryed (%d %d %d)", Location.x, Location.y, Location.z);
		}

	private:

		int UpdateConnections(ComponentFactory & factory, ComponentPtr down, ComponentPtr up)
		{
			// keep track of power levels from neighboring blocks as we connect to them
			int cp = 0;

			for (int i : GetLateralIndices())
			{
				auto side = ToSide(i);
				ComponentPtr comp = factory.GetComponent(side);
				if (comp == nullptr) // the side is air, check the block under
				{
					cp = std::max(cp, ConnectBlockDown(side, down, factory));
					cp = std::max(cp, ConnectBlockUp(side, up, factory));
				}
				else if (comp->IsFullBlock)
				{
					cp = std::max(cp, comp->CanStrongPower(this));
					cp = std::max(cp, ConnectBlockUp(side, up, factory));
					if (comp->CanConnect(this))
					{
						Connections.push_back(i);
					}
				}
				else
				{
					cp = std::max(cp, ConnectBlockDown(side, down, factory));
					cp = std::max(cp, comp->CanWeakPower(this));
					if (comp->CanConnect(this))
					{
						Connections.push_back(i);
					}
				}
			}

			// if we are only connected to a single side, that means the wire is leading into the block on the opposite side
			if (Connections.size() == 1)
			{
				int index = Connections.front();
				index += 2; // opposites are offest by 2
				index = (index % 4) + 4; // leading connections are always on the same y as the wire.
				Connections.push_back(index);
			}
			// if no connections, it means we can power all blocks around us
			else  if (Connections.size() == 0)
			{
				Connections = GetLateralIndices();
			}
			return cp;
		}

		int ConnectBlockDown(Vector3i side, ComponentPtr down, ComponentFactory & factory)
		{
			side.Move({ 0, -1, 0 });
			auto comp = factory.GetComponent(side);
			// we can only connect to wires this far away
			if (comp != nullptr && comp->Type == RedstoneType::WIRE)
			{
				// we are not on a slab so we don't form a connection to it, but can still draw from power down there.
				if (down != nullptr && down->IsFullBlock)
				{
					AddConnection(side);
				}
				return comp->CanWeakPower(this);
			}
			return 0;
		}

		int ConnectBlockUp(Vector3i side, ComponentPtr up, ComponentFactory & factory)
		{
			// to connect to a wire on the side, that is also up
			// there cannot be a solid block ontop of us
			if (up == nullptr || !up->IsFullBlock)
			{
				side.Move({ 0, 1, 0 });
				auto comp = factory.GetComponent(side);
				// we can only connect to wires this far away
				if (comp != nullptr && comp->Type == RedstoneType::WIRE)
				{
					AddConnection(side);
					return comp->CanWeakPower(this);
				}
			}
			return 0;
		}

		// instead of storing the 'real' vector to each connection, we'll store just an index to the side
		// we don't count top and bottom, as they are calculated differently
		// instead each side has three possible connections, level, up and down.
		inline Vector3i ToSide(int index) const
		{
			switch (index)
			{
				case 0: return Location + Vector3i{ -1, 1, 0 };
				case 1: return Location + Vector3i{ 0, 1, -1 };
				case 2: return Location + Vector3i{ 1, 1, 0 };
				case 3: return Location + Vector3i{ 0, 1, 1 };

				case 4: return Location + Vector3i{ -1, 0, 0 };
				case 5: return Location + Vector3i{ 0, 0, -1 };
				case 6: return Location + Vector3i{ 1, 0, 0 };
				case 7: return Location + Vector3i{ 0, 0, 1 };

				case 8: return Location + Vector3i{ -1, -1, 0 };
				case 9: return Location + Vector3i{ 0, -1, -1 };
				case 10: return Location + Vector3i{ 1, -1, 0 };
				case 11: return Location + Vector3i{ 0, -1, 1 };
			}
		}

		inline int ToIndex(Vector3i side) const
		{
			int index = 0;
			if (side.y == Location.y)
			{
				index += 4;
			}
			else if (side.y < Location.y)
			{
				index += 8;
			}

			if (side.z < Location.z)
			{
				index += 1;
			}
			else if (side.z > Location.z)
			{
				index += 3;
			}

			if (side.x > Location.x)
			{
				index += 2;
			}
			return index;
		}

		inline std::vector<int> GetLateralIndices() const
		{
			return{ 4, 5, 6, 7 };
		}

		inline void AddConnection(Vector3i side)
		{
			Connections.push_back(ToIndex(side));
		}

		inline bool HasConnection(Vector3i side)
		{
			int index = ToIndex(side);
			return find(begin(Connections), end(Connections), index) != end(Connections);
		}

		int power;
		int lastUpdateTick;

		// use ToIndex and ToSide to convert to vectors;
		std::vector<int> Connections;

	};
}