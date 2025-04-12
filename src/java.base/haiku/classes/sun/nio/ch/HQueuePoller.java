/*
 * Copyright (c) 2017, 2022, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */
package sun.nio.ch;

import java.io.IOException;
import static sun.nio.ch.HQueue.*;

/**
 * Poller implementation based on the HQueue (wait_for_objects) facility.
 */
class HQueuePoller extends Poller {
    private static final int MAX_EVENTS_TO_POLL = 512;

    private final int kqfd;
    private final int filter;
    private final long address;

    HQueuePoller(boolean read) throws IOException {
        super(read);
        throw new RuntimeException("Not implemented");
//        this.address = HQueue.allocatePollArray(MAX_EVENTS_TO_POLL);
    }

    @Override
    int fdVal() {
        throw new RuntimeException("Not implemented");
//        return kqfd;
    }

    @Override
    void implRegister(int fdVal) throws IOException {
        throw new RuntimeException("Not implemented");
    }

    @Override
    void implDeregister(int fdVal) {
        throw new RuntimeException("Not implemented");
    }

    @Override
    int poll(int timeout) throws IOException {
        throw new RuntimeException("Not implemented");    	
//        int n = HQueue.poll(kqfd, address, MAX_EVENTS_TO_POLL, timeout);
//        int i = 0;
//        while (i < n) {
//            long keventAddress = HQueue.getEvent(address, i);
//            int fdVal = HQueue.getDescriptor(keventAddress);
//            polled(fdVal);
//            i++;
//        }
//        return n;
    }
}
