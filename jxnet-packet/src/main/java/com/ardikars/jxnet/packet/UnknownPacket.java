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

package com.ardikars.jxnet.packet;

import com.ardikars.jxnet.util.ArrayUtils;
import com.ardikars.jxnet.util.HexUtils;

import java.util.Arrays;

/**
 * @author Ardika Rommy Sanjaya
 * @since 1.1.5
 */
public class UnknownPacket extends Packet {

    private byte[] data;

    public static UnknownPacket newInstance(final byte[] bytes) {
        return newInstance(bytes, 0, bytes.length);
    }

    public static UnknownPacket newInstance(final byte[] bytes, final int offset, final int length) {
        ArrayUtils.validateBounds(bytes, offset, length);
        UnknownPacket unknownPacket = new UnknownPacket();
        unknownPacket.setData(Arrays.copyOfRange(bytes, offset, length));
        return unknownPacket;
    }

    public byte[] getData() {
        return data;
    }

    public UnknownPacket setData(final byte[] data) {
        this.data = data;
        return this;
    }

    @Override
    public Packet setPacket(final Packet packet) {
        return this;
    }

    @Override
    public Packet getPacket() {
        return this;
    }

    @Override
    public byte[] toBytes() {
        return this.data;
    }

    @Override
    public Packet build() {
        return this;
    }

    @Override
    public String toString() {
        return HexUtils.toPrettyHexDump(getData());
    }

}