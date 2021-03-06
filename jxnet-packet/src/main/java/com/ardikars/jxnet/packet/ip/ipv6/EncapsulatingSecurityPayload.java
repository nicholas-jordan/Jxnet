/**
 * Copyright (C) 2017  Ardika Rommy Sanjaya
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.ardikars.jxnet.packet.ip.ipv6;

import com.ardikars.jxnet.packet.Packet;

import java.nio.ByteBuffer;

/**
 * @author Ardika Rommy Sanjaya
 * @since 1.1.5
 */
public class EncapsulatingSecurityPayload extends Packet {

    private static int FIXED_HEADER_LENGTH;

    private int securityParameterIndex;
    private int sequenceNumber;

    public int getSecurityParameterIndex() {
        return this.securityParameterIndex;
    }

    public EncapsulatingSecurityPayload setSecurityParameterIndex(final int securityParameterIndex) {
        this.securityParameterIndex = securityParameterIndex;
        return this;
    }

    public int getSequenceNumber() {
        return this.sequenceNumber;
    }

    public EncapsulatingSecurityPayload setSequenceNumber(final int sequenceNumber) {
        this.sequenceNumber = sequenceNumber;
        return this;
    }

    @Override
    public Packet setPacket(Packet packet) {
        return null;
    }

    @Override
    public Packet getPacket() {
        return null;
    }

    @Override
    public byte[] toBytes() {
        byte[] data = new byte[FIXED_HEADER_LENGTH];
        ByteBuffer buffer = ByteBuffer.wrap(data);
        buffer.putInt(this.getSecurityParameterIndex());
        buffer.putInt(this.getSequenceNumber());
        return data;
    }

    @Override
    public Packet build() {
        return this;
    }

    @Override
    public String toString() {
        return new StringBuilder("p")
                .append("[SPI: ")
                .append(this.getSecurityParameterIndex())
                .append(", Seq Num: ")
                .append(this.getSequenceNumber())
                .append("]").toString();
    }

}
