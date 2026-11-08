# UART protocol

115200 8N1 on `uart1`. Wire TX to RX crosswise and share ground. GP4 is TX, GP5 is RX, GP6 is the wake line (comms drives, control wakes on it).

Frame: `SOF 0xA5 | type | len | payload[len] | crc16`

CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF) over `type`, `len` and `payload`, sent big-endian. Max payload 32 bytes.

| Type | Name | Direction | Payload |
|---|---|---|---|
| 0x01 | HEARTBEAT | both | none, sent every 1 s |
| 0x10 | ALARM_START | comms to control | TBD |
| 0x11 | ALARM_STOP | either | none |
| 0x20 | STATE | control to comms | TBD |

Either side treats three missed heartbeats as a dead link.
