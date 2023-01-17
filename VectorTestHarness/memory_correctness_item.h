#pragma once

class MemoryCorrectnessItem
{
public:
    MemoryCorrectnessItem(int id = 0) : id(id)
    {
        if (memory_initialization_token == 0x2c1dd27f0d59cf3e && status != MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        status = MemoryStatus::Constructed;
        memory_initialization_token = 0x2c1dd27f0d59cf3e;
        count_constructed += 1;
    }

    MemoryCorrectnessItem(const MemoryCorrectnessItem& other) : id(other.id)
    {
        if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::MovedFrom)
        {
            errors_occurred += 1;
        }
        if (memory_initialization_token == 0x2c1dd27f0d59cf3e && status != MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        status = MemoryStatus::Constructed;
        memory_initialization_token = 0x2c1dd27f0d59cf3e;
        count_constructed_copy += 1;
    }

    MemoryCorrectnessItem(MemoryCorrectnessItem&& other) : id(other.id)
    {
        if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::MovedFrom)
        {
            errors_occurred += 1;
        }
        if (memory_initialization_token == 0x2c1dd27f0d59cf3e && status != MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        other.id = -1;
        other.status = MemoryStatus::MovedFrom;
        status = MemoryStatus::Constructed;
        memory_initialization_token = 0x2c1dd27f0d59cf3e;
        count_constructed_move += 1;
    }

    MemoryCorrectnessItem& operator=(const MemoryCorrectnessItem& other)
    {
        if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::MovedFrom)
        {
            errors_occurred += 1;
        }
        if (memory_initialization_token != 0x2c1dd27f0d59cf3e)
        {
            errors_occurred += 1;
        }
        id = other.id;
        count_assigned_copy += 1;
        return *this;
    }

    MemoryCorrectnessItem& operator=(MemoryCorrectnessItem&& other)
    {
        if (other.memory_initialization_token != 0x2c1dd27f0d59cf3e)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        if (other.status == MemoryStatus::MovedFrom)
        {
            errors_occurred += 1;
        }
        if (memory_initialization_token != 0x2c1dd27f0d59cf3e)
        {
            errors_occurred += 1;
        }
        id = other.id;
        other.id = -1;
        other.status = MemoryStatus::MovedFrom;
        count_assigned_move += 1;
        return *this;
    }

    ~MemoryCorrectnessItem()
    {
        if (memory_initialization_token != 0x2c1dd27f0d59cf3e)
        {
            errors_occurred += 1;
        }
        if (status == MemoryStatus::Deleted)
        {
            errors_occurred += 1;
        }
        status = MemoryStatus::Deleted;
        count_destroyed += 1;
    }

    int id;

    // Padding to push the memory_status back a little
    // Without this, some tests seem to generate false positive errors, which could be due to the memory at the start
    // of the object being reused for something else after deletion, changing the memory status while leaving the
    // token intact.
    char padding[16];

    enum class MemoryStatus
    {
        Uninitialized = 0,
        Constructed = 1,
        MovedFrom = 2,
        Deleted = 3
    };

    const char* MemoryStatusNames[4] = {
        "Uninitialized",
        "Constructed",
        "MovedFrom",
        "Deleted"
    };

    volatile MemoryStatus status;
    volatile uint64_t memory_initialization_token;

    static uint64_t count_alive()
    {
        return count_constructed + count_constructed_copy + count_constructed_move - count_destroyed;
    }

    static void reset()
    {
        count_constructed = 0;
        count_constructed_copy = 0;
        count_constructed_move = 0;
        count_assigned_copy = 0;
        count_assigned_move = 0;
        count_destroyed = 0;

        errors_occurred = 0;
    }

    static uint64_t count_constructed;
    static uint64_t count_constructed_copy;
    static uint64_t count_constructed_move;
    static uint64_t count_assigned_copy;
    static uint64_t count_assigned_move;
    static uint64_t count_destroyed;

    static uint64_t errors_occurred;
};
