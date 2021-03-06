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

package com.ardikars.jxnet;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Ardika Rommy Sanjaya
 * @since 1.1.4
 */
class Core {

    /**
     * Open a live capture from the network.
     * @param source interface name.
     * @param snaplen snaplen.
     * @param promisc promiscuous mode.
     * @param timeout timeout.
     * @param errbuf error buffer.
     * @return pcap object.
     */
    public static Pcap PcapOpenLive(PcapIf source, int snaplen, PromiscuousMode promisc,
                                    int timeout, StringBuilder errbuf) {
        return Jxnet.PcapOpenLive(source.getName(), snaplen, promisc.getValue(), timeout, errbuf);
    }

    /**
     * Is used to create a packet capture handle to look at packets on the network.
     * Source is a string that specifies the network device to open;
     * on Linux systems with 2.2 or later kernels, a source argument of "any" or NULL
     * can be used to capture packets from all interfaces.
     *
     * The returned handle must be activated with PcapActivate() before packets can be captured with it;
     * options for the capture, such as promiscuous mode, can be set on the handle before activating it.
     * @param source network device.
     * @param errbuf errof buffer.
     * @return returns a pcap_t * on success and NULL on failure. If NULL is returned, errbuf is filled in with an
     * appropriate error message.
     */
    public static Pcap PcapCreate(PcapIf source, StringBuilder errbuf) {
        return Jxnet.PcapCreate(source.getName(), errbuf);
    }

    /**
     * sets whether promiscuous mode should be set on a capture handle when the handle is activated.
     * If promisc is non-zero, promiscuous mode will be set, otherwise it will not be set.
     * @param pcap pcap object.
     * @param promiscuousMode promisc mode.
     * @return 0 on success.
     */
    public static int PcapSetPromisc(Pcap pcap, PromiscuousMode promiscuousMode) {
        return Jxnet.PcapSetPromisc(pcap, promiscuousMode.getValue());
    }

    /**
     * Sets whether immediate mode should be set on a capture handle when the handle is activated. If immediate_mode is non-zero,
     * immediate mode will be set, otherwise it will not be set.
     * @param pcap pcap object.
     * @param immediateMode immediate_mode.
     * @return 0 on success.
     */
    public static int PcapSetImmediateMode(Pcap pcap, ImmediateMode immediateMode) {
        return Jxnet.PcapSetImmediateMode(pcap, immediateMode.getValue());
    }

    /**
     * Compile a packet filter, converting an high level filtering expression
     * (see Filtering expression syntax) in a program that can be interpreted
     * by the kernel-level filtering engine.
     * @param pcap pcap object.
     * @param fp compiled bfp.
     * @param filter filter.
     * @param optimize optimize.
     * @param netmask netmask.
     * @return -1 on error, 0 otherwise.
     */
    public static int PcapCompile(Pcap pcap, BpfProgram fp, String filter,
                                  BpfProgram.BpfCompileMode optimize, Inet4Address netmask) {
        return Jxnet.PcapCompile(pcap, fp, filter, optimize.getValue(), netmask.toInt());
    }

    /**
     * Compile a packet filter without the need of opening an adapter.
     * This function converts an high level filtering expression (see Filtering expression syntax)
     * in a program that can be interpreted by the kernel-level filtering engine.
     * @param snaplen_arg snapshot length.
     * @param linkType datalink type.
     * @param program bpf.
     * @param buf filter.
     * @param optimize optimize.
     * @param netmask netmask.
     * @return -1 on error.
     */
    public static int PcapCompileNoPcap(int snaplen_arg, DataLinkType linkType, BpfProgram program,
                                  String buf, BpfProgram.BpfCompileMode optimize, Inet4Address netmask) {
        return Jxnet.PcapCompileNoPcap(snaplen_arg, linkType.getValue(), program, buf,
                optimize.getValue(), netmask.toInt());
    }

    /**
     * Return the link layer of an adapter.
     * @param pcap pcap object.
     * @return link layer of an adapter.
     */
    public static DataLinkType PcapDatalink(Pcap pcap) {
        return DataLinkType.valueOf((short) Jxnet.PcapDataLink(pcap));
    }

    /**
     * Set the current data link type of the pcap descriptor to the type
     * specified by dlt. -1 is returned on failure.
     * @param pcap pcap object.
     * @param linkType datalink type.
     * @return -1 on error, 0 otherwise.
     */
    public static int PcapSetDatalink(Pcap pcap, DataLinkType linkType) {
        return Jxnet.PcapSetDataLink(pcap, linkType.getValue());
    }

    /**
     * Create a pcap_t structure without starting a capture.
     * @param linkType datalink type.
     * @param snaplen snapshot length.
     * @return pcap object.
     */
    public static Pcap PcapOpenDead(DataLinkType linkType, int snaplen) {
        return Jxnet.PcapOpenDead(linkType.getValue(), snaplen);
    }

    /**
     * Removes all of the elements.
     * @param pcapIf PcapIf object.
     */
    public static void PcapFreeAllDevs(List<PcapIf> pcapIf) {
        if (!pcapIf.isEmpty()) {
            pcapIf.clear();
        }
    }

    /**
     * Return the first connected device to the network.
     * @param errbuf error buffer.
     * @return PcapIf object.
     */
    public static PcapIf LookupNetworkInterface(StringBuilder errbuf) {
        if (errbuf == null) throw new NullPointerException();
        List<PcapIf> pcapIfs = new ArrayList<>();
        if (Jxnet.PcapFindAllDevs(pcapIfs, errbuf) != Jxnet.OK) {
            return null;
        }
        for (PcapIf pcapIf : pcapIfs) {
            for (PcapAddr pcapAddr : pcapIf.getAddresses()) {
                if (pcapAddr.getAddr().getSaFamily() == SockAddr.Family.AF_INET) {
                    Inet4Address address = Inet4Address.valueOf(0);
                    Inet4Address netmask = Inet4Address.valueOf(0);
                    Inet4Address bcastaddr = Inet4Address.valueOf(0);
                    try {
                        address = Inet4Address.valueOf(pcapAddr.getAddr().getData());
                        netmask = Inet4Address.valueOf(pcapAddr.getNetmask().getData());
                        bcastaddr = Inet4Address.valueOf(pcapAddr.getBroadAddr().getData());
                        //Inet4Address dstaddr = Inet4Address.valueOf(pcapAddr.getDstAddr().getData());;
                    } catch (Exception e) {
                        errbuf.append(e.getMessage() + "\n");
                    }
                    if (!address.equals(Inet4Address.ZERO) && !address.equals(Inet4Address.LOCALHOST)
                            && !netmask.equals(Inet4Address.ZERO) && !bcastaddr.equals(Inet4Address.ZERO)) {
                        return pcapIf;
                    }
                }
            }
        }
        errbuf.append("Check your network connection.\n");
        return null;
    }

    /**
     * Select network interface.
     * @param errbuf errbuf.
     * @return PcapIf.
     */
    public static PcapIf SelectNetowrkInterface(StringBuilder errbuf) {
        if (errbuf == null) throw new NullPointerException();
        List<PcapIf> pcapIfs = new ArrayList<>();
        if (Jxnet.PcapFindAllDevs(pcapIfs, errbuf) != Jxnet.OK) {
            return null;
        }
        StringBuilder sb = new StringBuilder();
        int i=0;
        for (PcapIf pcapIf : pcapIfs) {
            sb.append("NO[" + ++i +"]\t=> ");
            sb.append("NAME: " + pcapIf.getName() + " (" + pcapIf.getDescription() + " )\n");
            for (PcapAddr pcapAddr : pcapIf.getAddresses()) {
                sb.append("\t\tADDRESS: " + pcapAddr.getAddr().toString() + "\n");
            }
        }
        System.out.println(sb.toString());
        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
        while (true) {
            System.out.print("Select a device number, or enter 'q' to quit -> ");
            String input;
            try {
                input = reader.readLine();
                i = Integer.parseInt(input);
            } catch (IOException e) {
                errbuf.append(e.toString());
                return null;
            }
            return pcapIfs.get(i-1);
        }
    }

}
