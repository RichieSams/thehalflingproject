/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/halfling_sys.h"
#include "common/linear_allocator.h"

struct ID3D11Device;
struct ID3D11DeviceContext;


namespace Graphics {

struct GraphicsState;

typedef void (*CommandExecuteFunctionPtr)(ID3D11Device *device, ID3D11DeviceContext *context, 
                                          BlendStateManager *blendStateManager, RasterizerStateManager *rasterizerStateManager, DepthStencilStateManager *depthStencilStateManager, 
                                          GraphicsState *currentGraphicsState, 
                                          const void *data);

struct CommandNode {
	CommandNode *NextNode;
	CommandExecuteFunctionPtr ExecuteFunction;
	CommandDisposeFunctionPtr DisposeFunction;
};

template <typename SortKeyType>
struct CommandPacket {
	CommandPacket()
		: Key(),
		FirstNode(nullptr) {}

	CommandPacket(SortKeyType key, CommandNode *node)
		: Key(key),
		FirstNode(node) {}

	SortKeyType Key;
	CommandNode *FirstNode;
};

template <typename SortKeyType>
bool CommandSortFunction(CommandPacket<SortKeyType> const& lhs, CommandPacket<SortKeyType> const& rhs) {
	return lhs.Key < rhs.Key;
}

/**
 * A bucket for sending graphics commands to. Submitted commands are not immediately
 * sent to the GPU. Rather, they are cached. When Submit() is called, the commands
 * are sorted and then sequentially sent to the GPU.
 *
 * NOTE: Commands can be grouped into 'packets' using AppendCommand(). The packet as
 * a whole will be sorted, but the order inside the packet will be preserved.
 */
template <typename SortKeyType, size_t Size>
class CommandBucket { 
public:
    /**
     * Create a new CommandBucket
	 *
	 * @tparam SortKeyType    The type of the key used to sort
     * @tparam Size           The maximum number of command 'packets' the bucket can store
     * @param  allocator      An allocator to use for internal memory allocations
	 *
	 * NOTE: T must have operator< implemented in order for the sort to function properly
     */
    CommandBucket(size_t allocatorPageSize) 
        : m_allocator(allocatorPageSize),
          m_nextFreeCommand(0u) {
    }
    
private:
    Common::LinearAllocator m_allocator;

	CommandPacket<SortKeyType> m_commands[Size];
    uint m_nextFreeCommand;
    
public:
    /**
     * Allocates a new command
	 * 
     * @tparam U      The type of the command to create. U must derive from 'CommandBase'
     * @param  key    The sort key for the new command
     * @return        The newly allocated command
     */
    template <typename U>
	U *AddCommand(SortKeyType key) {
		CommandNode *node = AllocateCommand<U>();

		// Store key and pointer to the node
		// TODO: Atomic this:
		uint currentPos = m_nextFreeCommand++;
		m_commands[currentPos].Key = key;
		m_commands[currentPos].FirstNode = node;

		return reinterpret_cast<U *>(GetCommandData(node));
	}

	/**
	 * Allocates a new command and appends it to the end of an existing command. The new command 'packet' is sorted as a whole
	 * entity and will execute the internal commands in the order they were added.
	 * 
	 * @tparam U                 The type of the command to create. U must derive from 'CommandBase'
	 * @param  previousCommand   The command to append the new command to
	 * @return                   The newly allocated command
	 */
	template <typename U>
	U *AppendCommand(void *previousCommand) {
		CommandNode *newNode = AllocateCommand<U>();

		CommandNode *previousNode = reinterpret_cast<CommandNode *>(reinterpret_cast<byte *>(previousCommand) - sizeof(CommandNode));
		// Make sure this command hasn't already been appended to
		AssertMsg(previousNode->NextNode == nullptr, "This Command has already had another command appended to it. Only append to the last Command created");
		previousNode->NextNode = newNode;

		return newNode->CommandData;
	}

	/**
	 * Sorts all the command packets and executes them in the sorted order
	 *
	 * @param device     The device to use for executing the commands
	 * @param context    the context to use for executing the commands
	 */
	void Submit(ID3D11Device *device, ID3D11DeviceContext *context) {
		// Sort the commands
		std::sort(std::begin(m_commands), std::end(m_commands), CommandSortFunction);

		// Execute the commands
		for (uint i = 0; i < m_nextFreeCommand; ++i) {
			CommandNode *node = m_commands[i].FirstNode;

			while (node->NextNode != nullptr) {
				node->ExecuteFunction(GetCommandData(node));
				node = node->NextNode;
			}
		}
	}
    
private:
    /**
     * A helper function to allocate a new CommandNode and initialize it
     *
     * @return        
     */
    template <typename U>
	CommandNode *AllocateCommand() {
		// We have to allocate enough room to fit all of the data of U. 
		CommandNode *newNode = reinterpret_cast<CommandNode *>(m_allocator.Allocate(sizeof(CommandNode) + sizeof(U)));
		newNode->NextNode = nullptr;
		newNode->ExecuteFunction = &U::Execute;

		return newNode;
	}

	/**
	 * A helper function to get the data for a command
	 *
	 * @param node    The CommandNode that is the header for the data
	 * @return        The data for the command
	 */
	inline static void *GetCommandData(CommandNode *node) {
		return reinterpret_cast<byte *>(node) + sizeof(CommandNode);
	}
};

}// End of namespace Graphics
