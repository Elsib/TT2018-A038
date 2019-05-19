 #include "stdint.h"
 #include "stdbool.h"

 #ifndef _C4GLTE_H_
 #define _C4GLTE_H_

 #define T_C4GLTE_P    const uint8_t*

 extern const uint8_t _C4GLTE_UNKNOWN ;                                   
 extern const uint8_t _C4GLTE_TEST    ;                                   
 extern const uint8_t _C4GLTE_SET     ;                                   
 extern const uint8_t _C4GLTE_GET     ;                                   
 extern const uint8_t _C4GLTE_URC     ;                                   
 extern const uint8_t _C4GLTE_EXEC    ;                                      
 extern const uint8_t _C4GLTE_EVARG_START_T    ;                          
 extern const uint8_t _C4GLTE_EVARG_END_T      ;                          
 extern const uint8_t _C4GLTE_EVARG_EVENT_T    ;                          
 extern const uint8_t _C4GLTE_EVENT_RESPONSE   ;                          
 extern const uint8_t _C4GLTE_EVENT_TIMEOUT    ;                          
 extern const uint8_t _C4GLTE_EVENT_BUFFER_OUT ;                          
 extern const uint8_t _C4GLTE_EVENT_CALLBACK   ;                          
 typedef void ( *T_c4glte_handler )( char *buffer, uint8_t *evArgs );

 #ifdef __cplusplus
 extern "C"{
 #endif

 void c4glte_uartDriverInit(T_C4GLTE_P gpioObj, T_C4GLTE_P uartObj);

 void c4glte_coreInit( T_c4glte_handler defaultHdl, uint32_t defaultWdog );

 void c4glte_hfcEnable( bool hfcState );

 void c4glte_modulePower( bool powerState );

 void c4glte_putc( char rxInput );

 void c4glte_tick();

 uint16_t c4glte_setHandler( char *pCmd, uint32_t timeout, T_c4glte_handler pHandler );

 void c4glte_cmdSingle( char *pCmd );

 void c4glte_cmdDouble( char *pCmd, char *pArg1 );

 void c4glte_cmdTriple( char *pCmd, char *pArg1, char *pArg2 );

 void c4glte_process();
 #ifdef __cplusplus
 } // extern "C"
 #endif
 #endif

 /* -------------------------------------------------------------------------- */
 /*
   __c4glte_driver.h

   Copyright (c) 2017, MikroElektonika - http://www.mikroe.com

   All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 3. All advertising materials mentioning features or use of this software
    must display the following acknowledgement:
    This product includes software developed by the MikroElektonika.

 4. Neither the name of the MikroElektonika nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY MIKROELEKTRONIKA ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL MIKROELEKTRONIKA BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ----------------------------------------------------------------------------- */