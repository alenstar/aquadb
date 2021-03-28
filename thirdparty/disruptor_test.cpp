#include <disruptorplus/ring_buffer.hpp>
#include <disruptorplus/multi_threaded_claim_strategy.hpp>
#include <disruptorplus/spin_wait_strategy.hpp>
#include <disruptorplus/sequence_barrier.hpp>

#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <cassert>

using namespace disruptorplus;
#include <disruptorplus/ring_buffer.hpp>
#include <disruptorplus/single_threaded_claim_strategy.hpp>
#include <disruptorplus/spin_wait_strategy.hpp>
#include <disruptorplus/sequence_barrier.hpp>

#include <thread>

using namespace disruptorplus;

struct Event
{
    uint32_t data;
};

int main()
{
    const size_t bufferSize = 1024; // Must be power-of-two

    ring_buffer<Event> buffer(bufferSize);

    spin_wait_strategy waitStrategy;
    single_threaded_claim_strategy<spin_wait_strategy> claimStrategy(bufferSize, waitStrategy);
    sequence_barrier<spin_wait_strategy> consumed(waitStrategy);
    claimStrategy.add_claim_barrier(consumed);

    std::thread consumer([&]()
    {
        uint64_t sum = 0;
        sequence_t nextToRead = 0;
        bool done = false;
        while (!done)
        {
            // Wait until more items available
            sequence_t available = claimStrategy.wait_until_published(nextToRead);

            // Process all available items in a batch
            do
            {
                auto& event = buffer[nextToRead];
                sum += event.data;
                if (event.data == 0)
                {
                    done = true;
                }
            } while (nextToRead++ != available);

            // Notify producer we've finished consuming some items
            consumed.publish(available);
        }
        std::cout << "sum is " << sum << std::endl;
    });

    std::thread producer([&]()
    {
        for (uint32_t i = 1; i <= 1000000; ++i)
        {
            // Claim a slot in the ring buffer, waits if buffer is full
            sequence_t seq = claimStrategy.claim_one();

            // Write to the slot in the ring buffer
            buffer[seq].data = i;

            // Publish the event to the consumer
            claimStrategy.publish(seq);
        }

        // Publish the terminating event.
        sequence_t seq = claimStrategy.claim_one();
        buffer[seq].data = 0;
        claimStrategy.publish(seq);
    });

    consumer.join();
    producer.join();

    return 0;
}
