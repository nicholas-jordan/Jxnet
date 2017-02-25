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

#include "../include/jxnet/com_ardikars_jxnet_Jxnet.h"

#include <pcap.h>
#include <string.h>
#include <dnet.h>
#include "../src/ids.h"
#include "../src/utils.h"
#include "preconditions.h"

#ifndef WIN32
#include <sys/socket.h>
#endif

/*
 * Class:     com_ardikars_jxnet_Jxnet
 * Method:    PcapFindAllDevs
 * Signature: (Ljava/util/List;Ljava/lang/StringBuilder;)I
 */
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapFindAllDevs
  (JNIEnv *env, jclass jcls, jobject jlist_pcap_if, jobject jerrbuf) {
  	/*if(jlist_pcap_if == NULL || jerrbuf == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return (jint) -1;
	}*/
	CheckNotNull(env, jlist_pcap_if, "");
	CheckNotNull(env, jerrbuf, "");
	
	SetListIDs(env);
	SetPcapIfIDs(env);
	SetPcapAddrIDs(env);
	SetSockAddrIDs(env);
	pcap_if_t *alldevsp = NULL;
	char errbuf[PCAP_ERRBUF_SIZE];
	int r = -1;
	errbuf[0] = '\0';
	r = pcap_findalldevs(&alldevsp, errbuf);
	if(r != 0 || alldevsp == NULL) {
		SetStringBuilder(env, jerrbuf, errbuf);
		return (jint) r;
	}
	jobject pcap_if = NULL;
	jobject pcap_addr = NULL;
	jobject list_pcap_addr = NULL;
	pcap_if_t *dev = alldevsp;
	pcap_addr_t *addr = NULL;
	while(dev != NULL) {
		pcap_if = NewObject(env, PcapIfClass, "<init>", "()V");
		if(dev->name != NULL) {
			(*env)->SetObjectField(env, pcap_if, PcapIfNameFID, (*env)->NewStringUTF(env, dev->name));
		} else {
			(*env)->SetObjectField(env, pcap_if, PcapIfNameFID, NULL);
		}
		if(dev->description != NULL) {
			(*env)->SetObjectField(env, pcap_if, PcapIfDescriptionFID, (*env)->NewStringUTF(env, dev->description));
		} else {
			(*env)->SetObjectField(env, pcap_if, PcapIfDescriptionFID, NULL);
		}
		list_pcap_addr = (*env)->GetObjectField(env, pcap_if, PcapIfAddressesFID);
		addr = dev->addresses;
		while(addr != NULL) {
			pcap_addr = NewObject(env, PcapAddrClass, "<init>", "()V");
			if(addr->addr != NULL) {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrAddrFID, NewSockAddr(env, addr->addr));
			} else {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrAddrFID, NewSockAddr(env, NULL));
			}
			if(addr->netmask != NULL) {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrNetmaskFID, NewSockAddr(env, addr->netmask));
			} else {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrNetmaskFID, NewSockAddr(env, NULL));
			}
			if(addr->broadaddr != NULL) {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrBroadAddrFID, NewSockAddr(env, addr->broadaddr));
			} else {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrBroadAddrFID, NewSockAddr(env, NULL));
			}
			if(addr->dstaddr != NULL) {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrDstAddrFID, NewSockAddr(env, addr->dstaddr));
			} else {
				(*env)->SetObjectField(env, pcap_addr, PcapAddrDstAddrFID, NewSockAddr(env, NULL));
			}
			if((*env)->CallBooleanMethod(env, list_pcap_addr, ListAddMID, pcap_addr) == JNI_FALSE) {
				(*env)->DeleteLocalRef(env, pcap_addr);
				return (jint) -1;
			}
			(*env)->DeleteLocalRef(env, pcap_addr);
			addr = addr->next;
		}
		(*env)->SetIntField(env, pcap_if, PcapIfFlagsFID, (jint) alldevsp->flags);
		if((*env)->CallBooleanMethod(env, jlist_pcap_if, ListAddMID, pcap_if) == JNI_FALSE) {
			(*env)->DeleteLocalRef(env, pcap_if);
			return (jint) -1;
		}
		(*env)->DeleteLocalRef(env, pcap_if);
		(*env)->DeleteLocalRef(env, list_pcap_addr);
		dev = dev->next;
	}
  	return (jint) r;
  }
  
/*
 * Class:     com_ardikars_jxnet_Jxnet
 * Method:    PcapOpenLive
 * Signature: (Ljava/lang/String;IIILjava/lang/StringBuilder;)Lcom/ardikars/jxnet/Pcap;
 */
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapOpenLive
  (JNIEnv *env, jclass jcls, jstring jsource, jint jsnaplen, jint jpromisc, jint jto_ms, jobject jerrbuf) {
  	/*if(jsource == NULL || jerrbuf == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return NULL;
  	}*/
	CheckNotNull(env, jsource, "");
	CheckArgument(env, (jsnaplen > 0 && jsnaplen < 65536 && (jpromisc == 0 || jpromisc == 1) &&
			jto_ms > 0), "");
	CheckNotNull(env, jerrbuf, "");

  	SetPointerIDs(env);
  	SetPcapIDs(env);
  	char errbuf[PCAP_ERRBUF_SIZE];
  	errbuf[0] = '\0';
  	const char *source = (*env)->GetStringUTFChars(env, jsource, 0);
  	pcap_t *pcap = pcap_open_live(source, (int) jsnaplen, (int) jpromisc, (int) jto_ms, errbuf);
  	(*env)->ReleaseStringUTFChars(env, jsource, source);
  	if(pcap == NULL) {
		SetStringBuilder(env, jerrbuf, errbuf);
		return NULL;
  	}
  	jobject obj = NewObject(env, PcapClass, "<init>", "()V");
  	(*env)->SetObjectField(env, obj, PcapPointerFID, SetPcap(env, pcap));
  	return obj;
  }
  
/*
 * Class:     com_ardikars_jxnet_Jxnet
 * Method:    PcapLoop
 * Signature: (Lcom/ardikars/jxnet/Pcap;ILcom/ardikars/jxnet/PcapHandler;Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapLoop
  (JNIEnv *env, jclass jcls, jobject jpcap, jint jcnt, jobject jcallback, jobject juser) {
 	/*if(jpcap == NULL || jcallback == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return -1;
	}*/
	CheckNotNull(env, jpcap,"");
	CheckNotNull(env, jcallback, "");
	CheckNotNull(env, juser, "");
	CheckArgument(env, (jcnt > 0), "");

	SetPointerIDs(env);
 	SetPcapIDs(env);
 	SetPcapPktHdrIDs(env);
 	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
 	if(pcap == NULL) {
 		return -1;
 	}
 	pcap_user_data_t user_data;
 	memset(&user_data, 0, sizeof(user_data));
 	user_data.env = env;
 	user_data.callback = jcallback;
 	user_data.user = juser;
 	user_data.PcapHandlerClass = (*env)->GetObjectClass(env, jcallback);
 	user_data.PcapHandlerNextPacketMID = (*env)->GetMethodID(env,
			user_data.PcapHandlerClass, "nextPacket", "(Ljava/lang/Object;Lcom/ardikars/jxnet/PcapPktHdr;Ljava/nio/ByteBuffer;)V");
	/*user_data.PcapPktHdrClass = PcapPktHdrClass;
	user_data.PcapPktHdrCaplenFID = PcapPktHdrCaplenFID;
	user_data.PcapPktHdrLenFID = PcapPktHdrLenFID;
	user_data.PcapPktHdrTvSecFID = PcapPktHdrTvSecFID;
	user_data.PcapPktHdrTvUsecFID = PcapPktHdrTvUsecFID;*/
  	return pcap_loop(pcap, (int) jcnt, pcap_callback, (u_char *) &user_data);
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDispatch
  (JNIEnv *env, jclass jcls, jobject jpcap, jint jcnt, jobject jcallback, jobject juser) {
 	/*if(jpcap == NULL || jcallback == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return (jint) -1;
	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jcallback, "");
	CheckNotNull(env, juser, "");
	CheckArgument(env, (jcnt > 0), "");

	SetPointerIDs(env);
 	SetPcapIDs(env);
 	SetPcapPktHdrIDs(env);
 	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
 	if(pcap == NULL) {
 		return (jint) -1;
 	}
 	pcap_user_data_t user_data;
 	memset(&user_data, 0, sizeof(user_data));
 	user_data.env = env;
 	user_data.callback = jcallback;
 	user_data.user = juser;
 	user_data.PcapHandlerClass = (*env)->GetObjectClass(env, jcallback);
 	user_data.PcapHandlerNextPacketMID = (*env)->GetMethodID(env,
			user_data.PcapHandlerClass, "nextPacket", "(Ljava/lang/Object;Lcom/ardikars/jxnet/PcapPktHdr;Ljava/nio/ByteBuffer;)V");
	return pcap_dispatch(pcap, (int) jcnt, pcap_callback, (u_char *) &user_data);
  }
  
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDumpOpen
  (JNIEnv *env, jclass jcls, jobject jpcap, jstring jfname) {
  	/*if(jpcap == NULL || jfname == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return NULL;
  	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jfname, "");

  	SetPointerIDs(env);
  	SetPcapIDs(env);
  	SetPcapDumperIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return NULL;
  	}
  	const char *fname = (*env)->GetStringUTFChars(env, jfname, 0);
  	pcap_dumper_t *pcap_dumper = pcap_dump_open(pcap, fname);
  	(*env)->ReleaseStringUTFChars(env, jfname, fname);
  	if(pcap_dumper == NULL) {
  		ThrowNew(env, PCAP_DUMPER_CLOSE_EXCEPTION, pcap_geterr(pcap));
  		return NULL;
  	}
  	jobject obj = NewObject(env, PcapDumperClass, "<init>", "()V");
  	(*env)->SetObjectField(env, obj, PcapDumperPointerFID, SetPcapDumper(env, pcap_dumper));
  	return obj;
  }
  
JNIEXPORT void JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDump
  (JNIEnv *env, jclass jcls, jobject jpcap_dumper, jobject jh, jobject jsp) {
  	/*if(jpcap_dumper == NULL || jh == NULL || jsp == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return;
  	}*/
  	//SetPcapDumperIDs(env);
  	//SetPcapPktHdrIDs(env);
	CheckNotNull(env, jpcap_dumper, "");
	CheckNotNull(env, jh, "");
	CheckNotNull(env, jsp, "");	

  	pcap_dumper_t *pcap_dumper = GetPcapDumper(env, jpcap_dumper);
  	if(pcap_dumper == NULL) {
		ThrowNew(env, PCAP_DUMPER_CLOSE_EXCEPTION, "");
		return;
  	}
	struct pcap_pkthdr hdr;
  	hdr.ts.tv_sec = (int) (*env)->GetIntField(env, jh, PcapPktHdrTvSecFID);
	hdr.ts.tv_usec = (int) (*env)->GetLongField(env, jh, PcapPktHdrTvUsecFID);
	hdr.caplen = (int) (*env)->GetIntField(env, jh, PcapPktHdrCaplenFID);
	hdr.len = (int) (*env)->GetIntField(env, jh, PcapPktHdrLenFID);
  	u_char *sp = (u_char *) (*env)->GetDirectBufferAddress(env, jsp);
  	pcap_dump((u_char *) pcap_dumper, &hdr, sp);
  }
  
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapOpenOffline
  (JNIEnv *env, jclass jcls, jstring jfname, jobject jerrbuf) {
  	/*if(jfname == NULL || jerrbuf == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return NULL;
  	}*/
	CheckNotNull(env, jfname, "");
	CheckNotNull(env, jerrbuf, "");

  	SetPointerIDs(env);
  	SetPcapIDs(env);
  	char errbuf[PCAP_ERRBUF_SIZE];
  	errbuf[0] = '\0';
  	const char *fname = (*env)->GetStringUTFChars(env, jfname, 0);
  	pcap_t *pcap = pcap_open_offline(fname, errbuf);
  	(*env)->ReleaseStringUTFChars(env, jfname, fname);
  	if(pcap == NULL) {
		SetStringBuilder(env, jerrbuf, errbuf);
		return NULL;
  	}
  	jobject obj = NewObject(env, PcapClass, "<init>", "()V");
  	(*env)->SetObjectField(env, obj, PcapPointerFID, SetPcap(env, pcap));
  	return obj;
  }

/*JNIEXPORT void JNICALL Java_com_ardikars_jxnet_BpfProgram_initBpfProgram
  (JNIEnv *env, jobject jobj) {
  	if (jobj == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return;
  	}
  	SetPointerIDs(env);
  	SetBpfProgramIDs(env);
  	jobject pointer = NewObject(env, PointerClass, "<init>", "()V");
  	struct bpf_program *fp = (struct bpf_program *) malloc(sizeof(struct bpf_program));
	if(fp == NULL) {
		ThrowNew(env, BPF_PROGRAM_CLOSE_EXCEPTION, "BpfProgram out of memory");
  		return;
	}
	fp->bf_insns = NULL;
	fp->bf_len = 0;
	(*env)->SetLongField(env, pointer, PointerAddressFID, PointerToJlong(fp));
	(*env)->SetObjectField(env, jobj, BpfProgramPointerFID, pointer);
	(*env)->DeleteLocalRef(env, pointer);
  }*/
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapCompile
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jfp, jstring jstr, jint joptimize, jint jnetmask) {
  	/*if (jpcap == NULL || jfp == NULL || jstr == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return (jint) -1;
  	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jfp, "");
	CheckNotNull(env, jstr, "");
	CheckArgument(env, (joptimize == 0 || joptimize == 1), "");

  	SetPointerIDs(env);
  	SetPcapIDs(env);
  	SetBpfProgramIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	struct bpf_program *fp = GetBpfProgram(env, jfp);
  	if(fp == NULL) {
  		ThrowNew(env, BPF_PROGRAM_CLOSE_EXCEPTION, pcap_geterr(pcap));
  		return (jint) -1;
  	}
  	const char *str = (*env)->GetStringUTFChars(env, jstr, 0);
  	int r = pcap_compile(pcap, fp, str, (int) joptimize, (bpf_u_int32) jnetmask);
  	(*env)->ReleaseStringUTFChars(env, jstr, str);
  	return r;
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapSetFilter
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jfp) {
  	/*if(jpcap == NULL || jfp == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return (jint) -1;
  	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jfp, "");

  	SetPointerIDs(env);
  	SetPcapIDs(env);
  	SetBpfProgramIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	struct bpf_program *fp = GetBpfProgram(env, jfp);
  	if(fp == NULL) {
  		ThrowNew(env, BPF_PROGRAM_CLOSE_EXCEPTION, pcap_geterr(pcap));
  		return (jint) -1;
  	}
  	return (jint) pcap_setfilter(pcap, fp);
  }
  
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapSendPacket
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jbuf, jint jsize) {
  	/*if(jpcap == NULL || jbuf == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return (jint) -1;
  	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jbuf, "");
	CheckArgument(env, (jsize > 0), "");

  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	const u_char *buf = (u_char *) (*env)->GetDirectBufferAddress(env, jbuf);
  	if(buf == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "Unable to retrive address from ByteBuffer");
  		return (jint) -1;
  	}
  	return (jint) pcap_sendpacket(pcap, buf + (int) 0, (int) jsize);
  }
  
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapNext
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jh) {
  	/*if(jpcap == NULL || jh == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return NULL;
  	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jh, "");

  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return NULL;
  	}
  	if(PcapPktHdrClass == NULL) { 
  		SetPcapPktHdrIDs(env);
  	}
  	struct pcap_pkthdr pkt_header;
  	const u_char *data = pcap_next(pcap, &pkt_header);
  	if(data != NULL) {
		(*env)->SetIntField(env, jh, PcapPktHdrCaplenFID, (jint) pkt_header.caplen);
		(*env)->SetIntField(env, jh, PcapPktHdrLenFID, (jint) pkt_header.len);
		(*env)->SetIntField(env, jh, PcapPktHdrTvSecFID, (jint) pkt_header.ts.tv_sec);
		(*env)->SetLongField(env, jh, PcapPktHdrTvUsecFID, (jlong) pkt_header.ts.tv_usec);
		return (*env)->NewDirectByteBuffer(env, (void *) data, pkt_header.caplen);
  	} else {
  		return NULL;
  	}
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapNextEx
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jpkt_header, jobject jpkt_data) {
  	/*if(jpcap == NULL || jpkt_header == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return -1;
  	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jpkt_header, "");
	CheckNotNull(env, jpkt_data, "");

  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return -1;
  	}
  	if(PcapPktHdrClass == NULL) { 
  		SetPcapPktHdrIDs(env);
  	}
  	if(ByteBufferClass == NULL) {
  		SetByteBufferIDs(env);
  	}
  	struct pcap_pkthdr *pkt_header;
  	const u_char *data = NULL;
  	int r = pcap_next_ex(pcap, &pkt_header, &data);
  	if(data != NULL) {
		(*env)->CallObjectMethod(env, jpkt_data, ByteBufferClearMID);
  		(*env)->CallObjectMethod(env, jpkt_data, ByteBufferPutMID, (*env)->NewDirectByteBuffer(env, (void *) data, pkt_header->caplen));
  		(*env)->SetIntField(env, jpkt_header, PcapPktHdrCaplenFID, (jint) pkt_header->caplen);
		(*env)->SetIntField(env, jpkt_header, PcapPktHdrLenFID, (jint) pkt_header->len);
		(*env)->SetIntField(env, jpkt_header, PcapPktHdrTvSecFID, (jint) pkt_header->ts.tv_sec);
		(*env)->SetLongField(env, jpkt_header, PcapPktHdrTvUsecFID, (jlong) pkt_header->ts.tv_usec);
  	}
  	return r;
  }

JNIEXPORT void JNICALL Java_com_ardikars_jxnet_Jxnet_PcapClose
  (JNIEnv *env, jclass jcls, jobject jpcap) {
  	/*if(jpcap == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return;
  	}*/
	CheckNotNull(env, jpcap, "");

  	SetPointerIDs(env);
  	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return;
  	}
  	pcap_close(pcap);
  	(*env)->SetObjectField(env, jpcap, PcapPointerFID, SetPcap(env, NULL));
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDumpFlush
  (JNIEnv *env, jclass jcls, jobject jpcap_dumper) {
  	/*if(jpcap_dumper == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return (jint) -1;
  	}*/
	CheckNotNull(env, jpcap_dumper, "");

  	SetPointerIDs(env);
  	SetPcapDumperIDs(env);
  	return (jint) pcap_dump_flush(GetPcapDumper(env, jpcap_dumper));
  }
  
JNIEXPORT void JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDumpClose
  (JNIEnv *env, jclass jcls, jobject jpcap_dumper) {
  	/*if(jpcap_dumper == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "");
  		return;
  	}*/
	CheckNotNull(env, jpcap_dumper, "");

  	SetPointerIDs(env);
  	SetPcapDumperIDs(env);
  	pcap_dump_close(GetPcapDumper(env, jpcap_dumper));
  	jobject pointer = NewObject(env, PointerClass, "<init>", "()V");
  	(*env)->SetLongField(env, pointer, PointerAddressFID, (jlong) 0);
	(*env)->SetObjectField(env, jpcap_dumper, PcapDumperPointerFID, pointer);
	(*env)->DeleteLocalRef(env, pointer);
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDatalink
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	CheckNotNull(env, jpcap, "");

	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	return (jint) pcap_datalink(pcap);
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapSetDatalink
  (JNIEnv *env, jclass jcls, jobject jpcap, jint jdtl) {
	CheckNotNull(env, jpcap, "");
	CheckArgument(env, (jdtl < 1), "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	return (jint) pcap_set_datalink(pcap, (int) jdtl);
  }
  
JNIEXPORT void JNICALL Java_com_ardikars_jxnet_Jxnet_PcapBreakLoop
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	CheckNotNull(env, jpcap, "");
  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return;
  	}
  	pcap_breakloop(pcap);
  }
  
JNIEXPORT jstring JNICALL Java_com_ardikars_jxnet_Jxnet_PcapLookupDev
  (JNIEnv *env, jclass jcls, jobject jerrbuf) {
	CheckNotNull(env, jerrbuf, "");

  	SetStringBuilderIDs(env);
  	char errbuf[PCAP_ERRBUF_SIZE];
  	errbuf[0] = '\0';
	const char *r = pcap_lookupdev(errbuf);
	if(r == NULL) {
		SetStringBuilder(env, jerrbuf, errbuf);
	}
#ifdef WIN32
	int size=WideCharToMultiByte(0, 0, (const WCHAR*) r, -1, NULL, 0, NULL, NULL);
	char device[size + 1];
	WideCharToMultiByte(0, 0, (const WCHAR*) r, -1, device, size, NULL, NULL);
	return (*env)->NewStringUTF(env, device);
#endif
	return (*env)->NewStringUTF(env, r);
  }
  
JNIEXPORT jstring JNICALL Java_com_ardikars_jxnet_Jxnet_PcapGetErr
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	CheckNotNull(env, jpcap, "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return NULL;
  	}
	return (jstring) (*env)->NewStringUTF(env, pcap_geterr(pcap));
  }
  
JNIEXPORT jstring JNICALL Java_com_ardikars_jxnet_Jxnet_PcapLibVersion
  (JNIEnv *env, jclass cls) {
  	return (*env)->NewStringUTF(env, pcap_lib_version());
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapIsSwapped
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	CheckNotNull(env, jpcap, "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	return (jint) pcap_is_swapped(pcap);
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapSnapshot
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	CheckNotNull(env, jpcap, "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	return (jint) pcap_snapshot(pcap);
  }
  
JNIEXPORT jstring JNICALL Java_com_ardikars_jxnet_Jxnet_PcapStrError
  (JNIEnv *env, jclass jcls, jint jerror) {
	CheckArgument(env, (jerror < 0), "");
  	return (*env)->NewStringUTF(env, pcap_strerror((int) jerror));
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapMajorVersion
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	CheckNotNull(env, jpcap, "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	return (jint) pcap_major_version(pcap);
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapMinorVersion
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	CheckNotNull(env, jpcap, "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	return (jint) pcap_minor_version(pcap);
  }
  
JNIEXPORT jstring JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDatalinkValToName
  (JNIEnv *env, jclass jcls, jint jdtl) {
	CheckArgument(env, (jdtl < 1), "");

  	return (*env)->NewStringUTF(env, (char *) pcap_datalink_val_to_name((jint) jdtl));
  }
  
JNIEXPORT jstring JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDatalinkValToDescription
  (JNIEnv *env, jclass jcls, jint jdtl) {
	CheckArgument(env, (jdtl < 1), "");

  	return (*env)->NewStringUTF(env, (char *) pcap_datalink_val_to_description((jint) jdtl));
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDatalinkNameToVal
  (JNIEnv *env, jclass jcls, jstring jname) {
	CheckNotNull(env, jname, "");
	
	const char *name = (*env)->GetStringUTFChars(env, jname, 0);
  	int r = pcap_datalink_name_to_val(name);
  	(*env)->ReleaseStringUTFChars(env, jname, name);
  	return r;
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapSetNonBlock
  (JNIEnv *env, jclass jcls, jobject jpcap, jint jnonblock, jobject jerrbuf) {
	CheckNotNull(env, jpcap, "");
	CheckArgument(env, (jnonblock == 0 || jnonblock == 1), "1 to enable non blocking, 0 otherwise.");
	CheckNotNull(env, jerrbuf, "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	char errbuf[PCAP_ERRBUF_SIZE];
  	errbuf[0] = '\0';
  	int r = pcap_setnonblock(pcap, (int) jnonblock, errbuf);
  	SetStringBuilder(env, jerrbuf, errbuf);
  	return (jint) r;
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapGetNonBlock
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jerrbuf) {
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jerrbuf, "");

  	SetPointerIDs(env);
	SetPcapIDs(env);
  	pcap_t *pcap = GetPcap(env, jpcap); // Exception already thrown
  	if(pcap == NULL) {
  		return (jint) -1;
  	}
  	char errbuf[PCAP_ERRBUF_SIZE];
  	errbuf[0] = '\0';
  	int r = pcap_getnonblock(pcap, errbuf);
  	return (jint) r;
  }
  
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapOpenDead
  (JNIEnv *env, jclass jcls, jint jlinktype, jint jsnaplen) {
	CheckArgument(env, (jlinktype < 1), "");
	CheckArgument(env, (jsnaplen > 0 || jsnaplen < 65535), "");

  	SetPointerIDs(env);
  	SetPcapIDs(env);
  	pcap_t *pcap = pcap_open_dead((int) jlinktype, (int) jsnaplen);
  	jobject obj = NewObject(env, PcapClass, "<init>", "()V");
  	(*env)->SetObjectField(env, obj, PcapPointerFID, SetPcap(env, pcap));
  	return obj;
  }

JNIEXPORT jlong JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDumpFTell
  (JNIEnv *env, jclass jcls, jobject jpcap_dumper) {
	/*if(jpcap_dumper == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
	}*/
	CheckNotNull(env, jpcap_dumper, "");

	SetPointerIDs(env);
	SetPcapDumperIDs(env);
	return (jlong) pcap_dump_ftell (GetPcapDumper(env, jpcap_dumper));
  }

JNIEXPORT void JNICALL Java_com_ardikars_jxnet_Jxnet_PcapFreeCode
  (JNIEnv *env, jclass jcls, jobject jfp) {
	/*if(jfp == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return;
	}*/
	CheckNotNull(env, jfp, "");

	SetPointerIDs(env);
	SetPcapIDs(env);
	SetBpfProgramIDs(env);
	struct bpf_program *fp = GetBpfProgram(env, jfp);
	if(fp == NULL) {
		ThrowNew(env, BPF_PROGRAM_CLOSE_EXCEPTION, "");
		return;
	}
	pcap_freecode(fp);
  }

JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapFile
  (JNIEnv *env, jclass jcls, jobject jpcap) {
	/*if(jpcap == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return NULL;
	}*/
	CheckNotNull(env, jpcap, "");

	SetPointerIDs(env);
	SetPcapIDs(env);
	SetFileIDs(env);
	FILE *file = pcap_file(GetPcap(env, jpcap));
	if(file == NULL) {
		return NULL;
	}
	jobject obj = NewObject(env, FileClass, "<init>", "()V");
	(*env)->SetObjectField(env, obj, FilePointerFID, SetFile(env, file));
	return obj;
  }

JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDumpFile
  (JNIEnv *env, jclass jcls, jobject jpcap_dumper) {
	/*if(jpcap_dumper == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return NULL;
	}*/
	CheckNotNull(env, jpcap_dumper, "");

	SetPointerIDs(env);
	SetPcapDumperIDs(env);
	SetFileIDs(env);
	FILE *file = pcap_dump_file(GetPcapDumper(env, jpcap_dumper));
	jobject obj = NewObject(env, FileClass, "<init>", "()V");
	(*env)->SetObjectField(env, obj, FilePointerFID, SetFile(env, file));
	return obj;
  }

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapStats
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jpcap_stat) {
	/*if(jpcap == NULL || jpcap_stat == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return -1;
	}*/
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jpcap_stat, "");

	SetPointerIDs(env);
	SetPcapIDs(env);
	SetPcapStatIDs(env);
	struct pcap_stat stats;
	memset(&stats, 0, sizeof(struct pcap_stat));
	int r = pcap_stats(GetPcap(env, jpcap), &stats);
	if(r == 0) {
		(*env)->SetLongField(env, jpcap_stat, PcapStatPsRecvFID, (jlong) stats.ps_recv);
		(*env)->SetLongField(env, jpcap_stat, PcapStatPsDropFID, (jlong) stats.ps_drop);
		(*env)->SetLongField(env, jpcap_stat, PcapStatPsIfDropFID, (jlong) stats.ps_ifdrop);
	}
	return r;
  }


/*

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Accept
  (JNIEnv *env, jclass jcls, jint jsocket, jobject jaddr, jint jaddrlen) {
	
	return -1;
}

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Listen
  (JNIEnv *env, jclass jcls, jint jsocket, jint jbacklog) {
	
	return -1;
}

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Bind
  (JNIEnv *env, jclass jcls, jobject jaddr, jint jaddrlen) {

	return -1;
}

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Connect
  (JNIEnv *env, jclass jcls, jint jsocket, jobject jaddr, jint jaddrlen) {
	
	return -1;
}

JNIEXPORT jint JNICALL Java_com_ardikars_

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Fork
  (JNIENv *env, jclass jcls) {
	
	return -1;
}

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Socket
  (JNIEnv *env, jclass jcls, jint jfa, jint jtype, jint jprotocol) {
  	return socket((int) jfa, (int) jtype, (int) jprotocol);
  }
  
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_RecvFrom
  (JNIEnv *env, jclass jcls, jint jsocket, jobject jbuf, jint len, jobject jsrc_addr, jint addrlen) {
// ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
	return -1;	
  }

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Recv
  (JNIEnv *env, jclass jcls, jint jsocket, jobject jbuf, jint len, jint jflags) {
// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
	return -1;
  }

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_RecvMsg
  (JNIEnv *env, jclass jcls, jint jsocket, jobject jmsghdr, jint jflags) {
// ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
	return -1;
}

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_Send
  (JNIEnv *env, jclass jcls, jint jsocket, jobject jbuf, jint jlen, jint jflags) {
	const u_char *buf = (u_char *) (*env)->GetDirectBufferAddress(env, jbuf);
	if(buf == NULL) {
                ThrowNew(env, NULL_PTR_EXCEPTION, "Unable to retrive address from ByteBuffer");
                return (jint) -1;
        }
	return send(jsocket, buf, jlen, jflags);
}

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_SendTO
  (JNIEnv *env, jclass jcls, jint jsocket, jobject jbuf, jint jlen, jint jflags, jint jsa_family, jbyteArray jsa_data, jint jto_len) {
  	const u_char *buf = (u_char *) (*env)->GetDirectBufferAddress(env, jbuf);
  	if(buf == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "Unable to retrive address from ByteBuffer");
  		return (jint) -1;
  	}
	if(jsa_data == NULL) {
		return -1;
	}
  	struct sockaddr sock;
  	jbyte *sa_data = (*env)->GetByteArrayElements(env, jsa_data, NULL);
 
  	sock.sa_family = jsa_family;
  	strcpy(sock.sa_data, (char *) sa_data);
  	int r = sendto((int) jsocket, buf, (int) jlen, (int) jflags, &sock, sizeof(sock));
  	(*env)->ReleaseByteArrayElements(env, jsa_data, sa_data, JNI_ABORT);
  	return r;
  }

*/

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapLookupNet
  (JNIEnv *env, jclass jclass, jstring jdevice, jobject jnetp, jobject jmaskp, jobject jerrbuf) {
	CheckNotNull(env, jdevice, "");
	CheckNotNull(env, jnetp, "");
	CheckNotNull(env, jmaskp, "");
	CheckNotNull(env, jerrbuf, "");

	char errbuf[PCAP_ERRBUF_SIZE];
	errbuf[0] = '\0';
	SetInet4AddressIDs(env);
	bpf_u_int32 netp = (bpf_u_int32)(*env)->CallIntMethod(env, jnetp, Inet4AddressToIntMID);
	bpf_u_int32 maskp = (bpf_u_int32)(*env)->CallIntMethod(env, jmaskp, Inet4AddressToIntMID);
	const char *device = (*env)->GetStringUTFChars(env, jdevice, 0);
	int r = pcap_lookupnet(device, &netp, &maskp, errbuf);
	(*env)->ReleaseStringUTFChars(env, jdevice, device);
	SetStringBuilder(env, jerrbuf, errbuf);
	swap_order_uint32(&netp);
	swap_order_uint32(&maskp);
	jobject netp_jobj = (*env)->CallStaticObjectMethod(env, Inet4AddressClass, Inet4AddressValueOfMID, (int) netp);
	jobject maskp_jobj = (*env)->CallStaticObjectMethod(env, Inet4AddressClass, Inet4AddressValueOfMID, (int) maskp);
	(*env)->CallVoidMethod(env, jnetp, Inet4AddressUpdateMID, netp_jobj);
	(*env)->CallVoidMethod(env, jmaskp, Inet4AddressUpdateMID, maskp_jobj);
	return r;
  }

JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_PcapCompileNoPcap
  (JNIEnv *env, jclass jclass, jint jsnaplen_arg, jint jlinktype_arg, jobject jprogram, jstring jbuf, jint joptimize, jint jmask) {
	CheckArgument(env, (jsnaplen_arg > 0 && jsnaplen_arg < 65535), "");
	CheckArgument(env, (jlinktype_arg < 1), "");
	CheckNotNull(env, jprogram, "");
	CheckNotNull(env, jbuf, "");
	CheckArgument(env, (joptimize == 0 || joptimize == 1), "");

	SetPointerIDs(env);
	SetBpfProgramIDs(env);
	struct bpf_program *program = GetBpfProgram(env, jprogram);
	if(program == NULL) {
		ThrowNew(env, BPF_PROGRAM_CLOSE_EXCEPTION, "");
		return (jint) -1;
	}
	const char *buf = (*env)->GetStringUTFChars(env, jbuf, 0);
	int r = pcap_compile_nopcap((int)jsnaplen_arg, (int) jlinktype_arg, program, buf, (int) joptimize, (bpf_u_int32) jmask);
	(*env)->ReleaseStringUTFChars(env, jbuf, buf);
	return r;
  }

JNIEXPORT void JNICALL Java_com_ardikars_jxnet_Jxnet_PcapPError
  (JNIEnv *env, jclass jclass, jobject jpcap, jstring jprefix) {
	CheckNotNull(env, jpcap, "");
	CheckNotNull(env, jprefix, "");

	SetPointerIDs(env);
	SetPcapIDs(env);
	const char *prefix =  (*env)->GetStringUTFChars(env, jprefix, 0);
	pcap_perror(GetPcap(env, jpcap), (char *) prefix);
	(*env)->ReleaseStringUTFChars(env, jprefix, prefix);
  }
/*
// Not supported in WinPcap
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_PcapDumpFOpen
  (JNIEnv *env, jclass jcls, jobject jpcap, jobject jf) {
	if(jpcap == NULL || jf == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return NULL;
	}
	SetPointerIDs(env);
	SetPcapIDs(env);
	SetFileIDs(env);
	SetPcapDumperIDs(env);
	pcap_dumper_t *pcap_dumper = pcap_dump_fopen(GetPcap(env, jpcap), GetFile(env, jf));
	if(pcap_dumper == NULL) {
		ThrowNew(env, PCAP_DUMPER_CLOSE_EXCEPTION, pcap_geterr(GetPcap(env, jpcap)));
		return NULL;
	}
	jobject obj = NewObject(env, PcapDumperClass, "<init>", "()V");
	(*env)->SetObjectField(env, obj, PcapDumperPointerFID, SetPcapDumper(env, pcap_dumper));
	return obj;
}
*/



/*
 *  * Class:     com_ardikars_jxnet_Jxnet
 *   * Method:    ArpOpen
 *    * Signature: ()Lcom/ardikars/jxnet/Arp;
 *     */
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_ArpOpen
  (JNIEnv *env, jclass jclass) {
	SetPointerIDs(env);
	SetArpIDs(env);
	arp_t *arp;
	if ((arp = arp_open()) == NULL) {
		return NULL;
	}
	jobject obj = NewObject(env, ArpClass, "<init>", "()V");
	(*env)->SetObjectField(env, obj, ArpPointerFID, SetArp(env, arp));
	return obj;
  }

/*
 * Class:     com_ardikars_jxnet_Jxnet
 * Method:    ArpClose
 * Signature: (Lcom/ardikars/jxnet/Arp;)Lcom/ardikars/jxnet/Arp;
 */
JNIEXPORT jobject JNICALL Java_com_ardikars_jxnet_Jxnet_ArpClose
  (JNIEnv *env, jclass jclass, jobject jarp) {
    CheckNotNull(env, jarp, "");

    if(jarp == NULL) {
  		ThrowNew(env, NULL_PTR_EXCEPTION, "Arp is closed.");
  		return NULL;
  	}
  	SetPointerIDs(env);
  	SetArpIDs(env);
  	arp_t *arp = GetArp(env, jarp); // Exception already thrown  	
    if(arp == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "Arp is closed.");
  	    return NULL;
    }
    arp_close(arp);
  	(*env)->SetObjectField(env, jarp, ArpPointerFID, SetArp(env, NULL));
  	return jarp;
  }

/*
 * Class:     com_ardikars_jxnet_Jxnet
 * Method:    ArpLoop
 * Signature: (Lcom/ardikars/jxnet/Arp;Lcom/ardikars/jxnet/ArpHandler;Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_ArpLoop
  (JNIEnv *env, jclass jclass, jobject jarp, jobject jcallback, jobject jarg) {
	CheckNotNull(env, jarp, "");
	CheckNotNull(env, jcallback, "");
	CheckNotNull(env, jarg, "");

	if(jarp == NULL || jcallback == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "");
		return -1;
	}
	SetPointerIDs(env);
 	SetArpIDs(env);
	SetArpEntryIDs(env);
	SetAddrIDs(env);
 	arp_t *arp = GetArp(env, jarp); // Exception already thrown
 	if(arp == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "Arp is closed.");
 		return -1;
 	}
    arp_user_data_t user_data;
 	memset(&user_data, 0, sizeof(user_data));
 	user_data.env = env;
 	user_data.callback = jcallback;
 	user_data.user = jarg;
 	user_data.ArpHandlerClass = (*env)->GetObjectClass(env, jcallback);
 	user_data.ArpHandlerNextArpEntryMID = (*env)->GetMethodID(env,
			user_data.ArpHandlerClass, "nextArpEntry", "(Lcom/ardikars/jxnet/ArpEntry;Ljava/lang/Object;)I");
  	return arp_loop(arp, arp_callback, (void *) &user_data);
  }

/*
 * Class:     com_ardikars_jxnet_Jxnet
 * Method:    ArpAdd
 * Signature: (Lcom/ardikars/jxnet/Arp;Lcom/ardikars/jxnet/ArpEntry;)I    
 */
#include <err.h>
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_ArpAdd
  (JNIEnv *env, jclass jclass, jobject jarp, jobject jarp_entry) {
	CheckNotNull(env, jarp, "");
	CheckNotNull(env, jarp_entry, "");

    SetPointerIDs(env);
 	SetArpIDs(env);
	SetArpEntryIDs(env);
	SetAddrIDs(env);
    arp_t *arp = GetArp(env, jarp); // Exception already thrown
 	if(arp == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "Arp is closed.");
 		return -1;
 	} 
    jobject arp_pa = (*env)->GetObjectField(env, jarp_entry, ArpEntryArpPaFID);
    if (arp_pa == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_pa is null.");
        return -1;
    }
    jobject arp_ha = (*env)->GetObjectField(env, jarp_entry, ArpEntryArpHaFID);
    if (arp_ha == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_ha is null..");
        return -1;
    }
    struct arp_entry entry;
    jstring pa = (*env)->CallObjectMethod(env, arp_pa, AddrGetStringAddressMID);
    if (pa == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_pa.data is null.");
        return -1;
    }
    jstring ha = (*env)->CallObjectMethod(env, arp_ha, AddrGetStringAddressMID);
    if (ha == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_ha.data is null.");
        return -1;
    }
    const char *str_pa = (*env)->GetStringUTFChars(env, pa, 0);
    const char *str_ha = (*env)->GetStringUTFChars(env, ha, 0);
    //printf("%s is at %s\n", str_pa, str_ha);
    if (addr_pton(str_pa, &entry.arp_pa) < 0 || addr_pton(str_ha, &entry.arp_ha) < 0) {
        (*env)->ReleaseStringUTFChars(env, pa, str_pa);
        (*env)->ReleaseStringUTFChars(env, ha, str_ha);
        (*env)->DeleteLocalRef(env, arp_pa);
        (*env)->DeleteLocalRef(env, arp_ha);
        (*env)->DeleteLocalRef(env, pa);
        (*env)->DeleteLocalRef(env, ha);
        return -1;
    }
    (*env)->ReleaseStringUTFChars(env, pa, str_pa);
    (*env)->ReleaseStringUTFChars(env, ha, str_ha);
    (*env)->DeleteLocalRef(env, arp_pa);
    (*env)->DeleteLocalRef(env, arp_ha);
    (*env)->DeleteLocalRef(env, pa);
    (*env)->DeleteLocalRef(env, ha);

    return arp_add(arp, &entry);
  }

/*
 *  * Class:     com_ardikars_jxnet_Jxnet
 *   * Method:    ArpDelete
 *    * Signature: (Lcom/ardikars/jxnet/Arp;Lcom/ardikars/jxnet/ArpEntry;)I
 *     */
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_ArpDelete
  (JNIEnv *env, jclass jclass, jobject jarp, jobject jarp_entry) {
	CheckNotNull(env, jarp, "");
	CheckNotNull(env, jarp_entry, "");    

	SetPointerIDs(env);
 	SetArpIDs(env);
	SetArpEntryIDs(env);
	SetAddrIDs(env);
    arp_t *arp = GetArp(env, jarp); // Exception already thrown
 	if(arp == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "Arp is closed.");
 		return -1;
 	} 
    jobject arp_pa = (*env)->GetObjectField(env, jarp_entry, ArpEntryArpPaFID);
    if (arp_pa == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_pa is null.");
        return -1;
    }
    struct arp_entry entry;
    jstring pa = (*env)->CallObjectMethod(env, arp_pa, AddrGetStringAddressMID);
    if (pa == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_pa.data is null.");
        return -1;
    }
    const char *str_pa = (*env)->GetStringUTFChars(env, pa, 0);
    if (addr_pton(str_pa, &entry.arp_pa) < 0) {
        (*env)->ReleaseStringUTFChars(env, pa, str_pa);
        (*env)->DeleteLocalRef(env, arp_pa);
        (*env)->DeleteLocalRef(env, pa);
        return -1;
    }
    (*env)->ReleaseStringUTFChars(env, pa, str_pa);
    (*env)->DeleteLocalRef(env, arp_pa);
    (*env)->DeleteLocalRef(env, pa);
    return arp_delete(arp, &entry);
  }

/*
 *  * Class:     com_ardikars_jxnet_Jxnet
 *   * Method:    ArpGet
 *    * Signature: (Lcom/ardikars/jxnet/Arp;Lcom/ardikars/jxnet/ArpEntry;)I
 *     */
JNIEXPORT jint JNICALL Java_com_ardikars_jxnet_Jxnet_ArpGet
  (JNIEnv *env, jclass jclass, jobject jarp, jobject jarp_entry) {
	CheckNotNull(env, jarp, "");
	CheckNotNull(env, jarp_entry, "");

    SetPointerIDs(env);
 	SetArpIDs(env);
	SetArpEntryIDs(env);
	SetAddrIDs(env);
    arp_t *arp = GetArp(env, jarp); // Exception already thrown
 	if(arp == NULL) {
		ThrowNew(env, NULL_PTR_EXCEPTION, "Arp is closed.");
 		return -1;
 	} 
    jobject arp_pa = (*env)->GetObjectField(env, jarp_entry, ArpEntryArpPaFID);
    if (arp_pa == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_pa is null.");
        return -1;
    }
    struct arp_entry entry;
    jstring pa = (*env)->CallObjectMethod(env, arp_pa, AddrGetStringAddressMID);
    if (pa == NULL) {
        ThrowNew(env, NULL_PTR_EXCEPTION, "ArpEntry.arp_pa.data is null.");
        return -1;
    }
    const char *str_pa = (*env)->GetStringUTFChars(env, pa, 0);
    //printf("ip : %s\n", str_pa);
    if (addr_pton(str_pa, &entry.arp_pa) < 0) {
        //(*env)->CallStaticObjectMethod(env, ArpEntryClass, ArpEntryInitializeMID, NULL, NULL, NULL);
        (*env)->ReleaseStringUTFChars(env, pa, str_pa);     
        return -1;
    }
    int r = arp_get(arp, &entry);
    jobject arp_ha;
    jbyteArray jb = (*env)->NewByteArray(env, 4);
    (*env)->SetByteArrayRegion(env, jb, 0, 4, (jbyte *) &entry.arp_pa.addr_data8);
    arp_pa = (*env)->CallStaticObjectMethod(env, AddrClass, AddrInitializeMID,
            entry.arp_pa.addr_type, entry.arp_pa.addr_bits, jb);
    jb = (*env)->NewByteArray(env, 6);
    (*env)->SetByteArrayRegion(env, jb, 0, 6, (jbyte *) &entry.arp_ha.addr_data8);
    arp_ha = (*env)->CallStaticObjectMethod(env, AddrClass, AddrInitializeMID,
            entry.arp_ha.addr_type, entry.arp_ha.addr_bits, jb);
    
    (*env)->SetObjectField(env, jarp_entry, ArpEntryArpPaFID, arp_pa);
    (*env)->SetObjectField(env, jarp_entry, ArpEntryArpHaFID, arp_ha);
    
    (*env)->ReleaseStringUTFChars(env, pa, str_pa);
    (*env)->DeleteLocalRef(env, arp_pa);
    (*env)->DeleteLocalRef(env, arp_ha);
    (*env)->DeleteLocalRef(env, pa);
    return r;
  }
