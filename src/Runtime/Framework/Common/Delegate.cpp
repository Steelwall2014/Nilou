#include <atomic>

#include "Delegate.h"

namespace nilou {

    std::atomic<uint64> GNextID(1);

    uint64 FDelegateHandle::GenerateNewID()
    {
        // Just increment a counter to generate an ID.
        uint64 Result = ++GNextID;

        // Check for the next-to-impossible event that we wrap round to 0, because we reserve 0 for null delegates.
        if (Result == 0)
        {
            // Increment it again - it might not be zero, so don't just assign it to 1.
            Result = ++GNextID;
        }

        return Result;
    }
}