/*
 * Copyright (c) 2017, CATIE, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed.h"
#include "rtos.h"

namespace {
#define PERIOD_MS 500
}

static DigitalOut led1(LED1);
Thread thread_ping;
Thread thread_pong;
Mutex mutex;
Semaphore semaphore(2);
// main() runs in its own thread in the OS
// (note the calls to Thread::wait below for delays)

void ping()
    {

	int cpt = 0;

	while(cpt < 10){
		mutex.lock();
    	printf("Ping\n");
    	semaphore.release();
    	mutex.unlock();
    	cpt++;
		}
	}

void pong()
{

	int cpt = 0;

	while(cpt < 10){
		mutex.lock();
		semaphore.wait();
		printf("Pong\n");
		wait(1);
		mutex.unlock();
	cpt++;;
	}
}

int main()
{
	thread_ping.start(ping);
	thread_pong.start(pong);
	thread_ping.join();
	thread_pong.join();
	printf("Terminer");
}




