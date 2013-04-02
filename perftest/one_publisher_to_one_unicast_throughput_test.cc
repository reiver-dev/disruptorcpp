// Copyright (c) 2011, François Saint-Jacques
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the disruptor-- nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL FRANÇOIS SAINT-JACQUES BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <sys/time.h>

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

#include <disruptor.hpp>
#include "../test/support/stub_event.h"

typedef RingBuffer<StubEvent, SingleThreadClaimStrategy, YieldStrategy> RingBuffer_t;
typedef BatchEventProcessor<RingBuffer_t> EventProcessor_t;


int main(int arc, char** argv) {
    int buffer_size = 1024 * 8;
    long iterations = 1000L * 1000L * 300;

	RingBuffer_t ring_buffer(buffer_size, StubEventFactory());

    std::vector<Sequence*> sequence_to_track(0);
    SequenceBarrier barrier(ring_buffer, sequence_to_track);

    StubBatchHandler stub_handler;
    //IgnoreExceptionHandler<StubEvent> stub_exception_handler;
    EventProcessor_t processor(&ring_buffer, &barrier, &stub_handler);

    std::thread consumer(std::ref<EventProcessor_t>(processor));

    struct timeval start_time, end_time;

    gettimeofday(&start_time, NULL);

    StubEventTranslator translator;
    for (long i=0; i<iterations; i++) {
    	ring_buffer.publishEvent(translator);
    }

    long expected_sequence = ring_buffer.getCursor();
    while (processor.getSequence()->get() < expected_sequence) {}

    gettimeofday(&end_time, NULL);

    double start, end;
    start = start_time.tv_sec + ((double) start_time.tv_usec / 1000000);
    end = end_time.tv_sec + ((double) end_time.tv_usec / 1000000);

    std::cout.precision(15);
    std::cout << "1P-1EP-UNICAST performance: ";
    std::cout << (iterations * 1.0) / (end - start)
              << " ops/secs" << std::endl;

    processor.halt();
    consumer.join();

    return EXIT_SUCCESS;
}

