package com.ardikars.test;

import com.ardikars.jxnet.*;
import com.ardikars.jxnet.packet.ethernet.Ethernet;
import com.ardikars.jxnet.packet.ip.IPv4;
import com.ardikars.jxnet.packet.ip.IPv6;
import com.ardikars.jxnet.packet.tcp.TCP;
import com.ardikars.jxnet.packet.tcp.TCPFlags;
import com.ardikars.jxnet.util.HexUtils;

import java.nio.ByteBuffer;

public class ChecksumTest {

    private static String HEX_STREAM = "" +
            "4c5e0c78508f40f02fa46abe08004500003497d8400040064fe3ac1001e59df00723d76201bb9439b553e802b0c6801000e5816c00000101080a000b228e5c026753";

    public static void main(String[] args) {
        Ethernet ethernet = new Ethernet()
                .setDestinationMacAddress(MacAddress.DUMMY)
                .setSourceMacAddress(MacAddress.valueOf("AA:AA:AA:BB:BB:BB"));

        IPv6 iPv6 = new IPv6()
                .setDestinationAddress(Inet6Address.LOCALHOST)
                .setSourceAddress(Inet6Address.LOCALHOST);

        IPv4 iPv4 = new IPv4();
        iPv4.setIdentification((short) 15326);
        iPv4.setFlags((byte) 0x02);
        iPv4.setTtl((byte) 64);
        iPv4.setSourceAddress(Inet4Address.LOCALHOST);
        iPv4.setDestinationAddress(Inet4Address.LOCALHOST);

        TCP tcp = new TCP()
                .setSourcePort((short) 38948)
                .setDestinationPort((short) 443)
                .setSequence(0x665426d5)
                .setAcknowledge(0)
                .setFlags(TCPFlags.newInstance((short) 2))
                .setWindowSize((short) 29200)
                .setUrgentPointer((short) 0)
                .setOptions(HexUtils.parseHex("0101080a000b228e5c026753"));

        System.out.println(HexUtils.toHexString(tcp.toBytes()));

        iPv4.setPacket(tcp);
        ethernet.setPacket(iPv4);

        byte[] bytes = ethernet.toBytes();
        ByteBuffer buffer = ByteBuffer.allocateDirect(bytes.length);
        buffer.put(bytes);

        Pcap pcap = Jxnet.PcapOpenDead(1, 1500);
        PcapDumper dumper = Jxnet.PcapDumpOpen(pcap, "/tmp/test.pcap");
        PcapPktHdr pktHdr = new PcapPktHdr(buffer.capacity(), buffer.capacity(), 0, 0);
        Jxnet.PcapDump(dumper, pktHdr, buffer);
        Jxnet.PcapClose(pcap);


    }

}
