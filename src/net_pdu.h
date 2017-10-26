/*
 * net_pdu.h
 *
 *  Created on: 24 окт. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_NET_PDU_H_
#define SRC_NET_PDU_H_

#define PDU_POP(data, pdu, ofs, data_size) \
        do { \
            memcpy((data), &(pdu)[(ofs)], (data_size)); \
            (ofs) += (data_size); \
        } while (0);

#define PDU_POP_PROTECT(ofs, data_size, buf_len) \
        do { \
            if( (ofs) + (data_size) > (buf_len)) return -1; \
        } while (0)

#define PDU_POP_BUF(data, data_size) \
        do { \
            PDU_POP_PROTECT(ofs, (data_size), buf_len); \
            PDU_POP((data), buf, ofs, (data_size)); \
        } while (0)



#define PDU_PUSH(buf, ofs, data, data_size) \
        do { \
            memcpy(&(buf)[(ofs)], (data), (data_size)); \
            (ofs) += (data_size); \
        } while (0)

#define PDU_PUSH_PROTECT(ofs, data_size, buf_size) \
        do { \
            if( (ofs) + (data_size) > (buf_size)) return -1; \
        } while (0)

#define PDU_PUSH_BUF(data, data_size) \
        do { \
            PDU_PUSH_PROTECT(ofs, (data_size), buf_size); \
            PDU_PUSH(buf, ofs, (data), (data_size)); \
            *buf_len = ofs; \
        } while (0)


#endif /* SRC_NET_PDU_H_ */
