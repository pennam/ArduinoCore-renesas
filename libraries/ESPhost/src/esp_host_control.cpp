#include "esp_host_control.h"

#include "CCtrlWrapper.h"
#define MORE_FRAGMENT (1 << 0)
static CMsg cumulative_msg;

#define MAX_INTERFACE_LEN            IFNAMSIZ
#define MAC_SIZE_BYTES               6
#define MIN_MAC_STR_LEN              17


#define SUCCESS                      0
#define FAILURE                      -1
#define MAX_SSID_LENGTH              32
#define MIN_PWD_LENGTH               8
#define MAX_PWD_LENGTH               64
#define STATUS_LENGTH                14
#define TIMEOUT_PSERIAL_RESP         30
#define MIN_CHNL_NO                  1
#define MAX_CHNL_NO                  11
#define MIN_CONN_NO                  1
#define MAX_CONN_NO                  10


#define CTRL_LIB_STATE_INACTIVE      0
#define CTRL_LIB_STATE_INIT          1
#define CTRL_LIB_STATE_READY         2


#define CLEANUP_APP_MSG(app_msg) do {                                         \
  if (app_msg) {                                                              \
    if (app_msg->free_buffer_handle) {                                        \
      if (app_msg->free_buffer_func) {                                        \
        app_msg->free_buffer_func(app_msg->free_buffer_handle);               \
        app_msg->free_buffer_handle = NULL;                                   \
      }                                                                       \
    }                                                                         \
    mem_free(app_msg);                                                        \
  }                                                                           \
} while(0);


#define CHECK_CTRL_MSG_NON_NULL_VAL(msGparaM, prinTmsG)                       \
    if (!msGparaM) {                                                          \
        command_log(prinTmsG"\n");                                            \
        goto fail_parse_ctrl_msg;                                             \
    }

#define CHECK_CTRL_MSG_NON_NULL(msGparaM)                                     \
    if (!ctrl_msg->msGparaM) {                                                \
        command_log("Failed to process rx data\n");                           \
        goto fail_parse_ctrl_msg;                                             \
    }

#define CHECK_CTRL_MSG_FAILED(msGparaM)                                       \
    if (ctrl_msg->msGparaM->resp) {                                           \
        command_log("Failure resp/event: possibly precondition not met\n");   \
        goto fail_parse_ctrl_msg;                                             \
    }

#define CTRL_ALLOC_ASSIGN(TyPe,MsG_StRuCt)                                    \
    TyPe *req_payload = (TyPe *)                                              \
        hosted_calloc(1, sizeof(TyPe));                                       \
    if (!req_payload) {                                                       \
      command_log("Failed to allocate memory for req.%s\n",#MsG_StRuCt);      \
      failure_status = CTRL_ERR_MEMORY_FAILURE;                               \
        break;                                                                \
    }                                                                         \
    req.MsG_StRuCt = req_payload;                                             \
    buff_to_free1 = (uint8_t*)req_payload;



static ctrl_cmd_t answer;

/* #############
 *  PARSE EVENT
 * ############# */

static int esp_host_parse_event(CtrlMsg *ctrl_msg) {
   int rv = FAILURE;

   if (!ctrl_msg) {
      return FAILURE;
   }   

   memset(&answer,0x00, sizeof(ctrl_cmd_t));

   answer.msg_type          = CTRL_EVENT;
   answer.msg_id            = ctrl_msg->msg_id;
   answer.resp_event_status = SUCCESS;

   switch (ctrl_msg->msg_id) {
      case CTRL_EVENT_ESP_INIT: 
         rv = SUCCESS;  
         break;
      case CTRL_EVENT_HEARTBEAT: 
         if(ctrl_msg->event_heartbeat) {
            answer.u.e_heartbeat.hb_num = ctrl_msg->event_heartbeat->hb_num;
            rv = SUCCESS;
         }
         break;
      case CTRL_EVENT_STATION_DISCONNECT_FROM_AP: 
         if(ctrl_msg->event_station_disconnect_from_ap) {
            answer.resp_event_status = ctrl_msg->event_station_disconnect_from_ap->resp;
            rv = SUCCESS;
         } 
         break;
      case CTRL_EVENT_STATION_DISCONNECT_FROM_ESP_SOFTAP: 
         if(ctrl_msg->event_station_disconnect_from_esp_softap) {
            answer.resp_event_status = ctrl_msg->event_station_disconnect_from_esp_softap->resp;
            if(answer.resp_event_status == SUCCESS) {
               if(ctrl_msg->event_station_disconnect_from_esp_softap->mac.data) {
                  strncpy(answer.u.e_sta_disconnected.mac,(char *)ctrl_msg->event_station_disconnect_from_esp_softap->mac.data, ctrl_msg->event_station_disconnect_from_esp_softap->mac.len);
                  rv = SUCCESS;
               }
            }
         }
         break;
      default: 
         break;
   }

   return rv;
}

/* ################
 *  PARSE RESPONSE
 * ################ */
static int esp_host_parse_response(CtrlMsg *ctrl_msg) {
   int rv = FAILURE;

   if (!ctrl_msg) {
      return FAILURE;
   } 

   memset(&answer,0x00, sizeof(ctrl_cmd_t));

   answer.msg_type          = CTRL_EVENT;
   answer.msg_id            = ctrl_msg->msg_id;

   uint16_t i = 0;

   

   /* 3. parse CtrlMsg into ctrl_cmd_t */
   switch (ctrl_msg->msg_id) {
        case CTRL_RESP_CONFIG_HEARTBEAT: {
         CHECK_CTRL_MSG_NON_NULL(resp_config_heartbeat);
         CHECK_CTRL_MSG_FAILED(resp_config_heartbeat);
         break;
      } default: {
         command_log("Unsupported Control Resp[%u]\n", ctrl_msg->msg_id);
         goto fail_parse_ctrl_msg;
         break;
      }
   }

   /* 4. Free up buffers */
   //ctrl_msg__free_unpacked(ctrl_msg, NULL);
   //ctrl_msg = NULL;
   answer.resp_event_status = SUCCESS;
   return SUCCESS;

   /* 5. Free up buffers in failure cases */
fail_parse_ctrl_msg:
   //ctrl_msg__free_unpacked(ctrl_msg, NULL);
   //ctrl_msg = NULL;
   answer.resp_event_status = FAILURE;
   return FAILURE;

fail_parse_ctrl_msg2:
   //ctrl_msg__free_unpacked(ctrl_msg, NULL);
   //ctrl_msg = NULL;
   return FAILURE;
}





static int esp_host_process_ctrl_answer(CtrlMsg *ans) {
   
}





/* 
 * Function for CONTROL messages
 * This function gets a CMsg, verify the correctness of the tlv header (remember
 * that the payload header is verified when the message is put into the
 * received message queue, so no need to verify it here)
 * It returns a CtrlMsg if all is good or nullptr otherwise
 * REMEMBER: the CtrlMsg must be deleted!
 * 
 */
static CtrlMsg *esp_host_get_ctrl_msg(CMsg& msg) {
   
}









int esp_host_msg_received(ctrl_cmd_t **response) {
   int rv = 0;
  
   
   return rv;

#ifdef TO_BE_REMOVED
   /* process received buffer for all possible interface types */
      if (buf_handle.if_type == ESP_SERIAL_IF) {

         /* serial interface path */
         serial_rx_handler(&buf_handle);

      } else if((buf_handle.if_type == ESP_STA_IF) ||
            (buf_handle.if_type == ESP_AP_IF)) {
         priv = get_priv(buf_handle.if_type, buf_handle.if_num);

         if (priv) {
            buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
            assert(buffer);

            buffer->len = buf_handle.payload_len;
            buffer->payload = malloc(buf_handle.payload_len);
            assert(buffer->payload);

            memcpy(buffer->payload, buf_handle.payload,
                  buf_handle.payload_len);

            netdev_rx(priv->netdev, buffer);
         }

      } else if (buf_handle.if_type == ESP_PRIV_IF) {
         buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
         assert(buffer);

         buffer->len = buf_handle.payload_len;
         buffer->payload = malloc(buf_handle.payload_len);
         assert(buffer->payload);

         memcpy(buffer->payload, buf_handle.payload,
               buf_handle.payload_len);

         process_priv_communication(buffer);
         /* priv transaction received */
         printf("Received INIT event\n\r");

         event = (struct esp_priv_event *) (payload);
         if (event->event_type == ESP_PRIV_EVENT_INIT) {
            /* halt spi transactions for some time,
             * this is one time delay, to give breathing
             * time to slave before spi trans start */
            stop_spi_transactions_for_msec(50000);
            if (spi_drv_evt_handler_fp) {
               spi_drv_evt_handler_fp(TRANSPORT_ACTIVE);
            }
         } else {
            /* User can re-use this type of transaction */
         }
      } else if (buf_handle.if_type == ESP_TEST_IF) {
#if TEST_RAW_TP
         update_test_raw_tp_rx_len(buf_handle.payload_len);
#endif
      } else {
         printf("unknown type %d \n\r", buf_handle.if_type);
      }

      /* Free buffer handle */
      /* When buffer offloaded to other module, that module is
       * responsible for freeing buffer. In case not offloaded or
       * failed to offload, buffer should be freed here.
       */
      if (buf_handle.free_buf_handle) {
         buf_handle.free_buf_handle(buf_handle.priv_buffer_handle);
      }
   }
   #endif

}

extern void esp_host_spi_transaction(void);

int esp_host_wait_for_answer(ctrl_cmd_t *req) {
  


}




/* i.e. original "ctrl_app_send_req" */
int esp_host_ctrl_send_req(ctrl_cmd_t *app_req) {
   
   int       ret = SUCCESS;
   CtrlMsg   req = {0};
   uint32_t  tx_protobuf_len = 0;
   uint8_t   failure_status = 0;
   uint8_t  *buff_to_free1 = NULL;
   void     *buff_to_free2 = NULL;

   if (!app_req) {
      failure_status = CTRL_ERR_INCORRECT_ARG;
      return FAILURE;
   }

   app_req->msg_type = CTRL_REQ;
   
   /* 2. Protobuf msg init */
   ctrl_msg__init(&req);
   
   req.msg_id = (CtrlMsgId)app_req->msg_id;
   /* payload case is exact match to msg id in esp_hosted_config.pb-c.h */
   req.payload_case = (CtrlMsg__PayloadCase) app_req->msg_id;

   /* 3. identify request and compose CtrlMsg */
   switch(req.msg_id) {
      
      case CTRL_REQ_GET_AP_CONFIG:
      case CTRL_REQ_DISCONNECT_AP:
      case CTRL_REQ_GET_SOFTAP_CONFIG:
      case CTRL_REQ_GET_SOFTAP_CONN_STA_LIST:
      case CTRL_REQ_STOP_SOFTAP:
      case CTRL_REQ_GET_PS_MODE:
      case CTRL_REQ_OTA_BEGIN:
      case CTRL_REQ_OTA_END:
      case CTRL_REQ_GET_WIFI_CURR_TX_POWER: 
         /* Intentional fallthrough & empty */
         break;
       
      
      case CTRL_REQ_SET_SOFTAP_VND_IE: {
         wifi_softap_vendor_ie_t *p = &app_req->u.wifi_softap_vendor_ie;
         CTRL_ALLOC_ASSIGN(CtrlMsgReqSetSoftAPVendorSpecificIE,
               req_set_softap_vendor_specific_ie);

         if ((p->type > WIFI_VND_IE_TYPE_ASSOC_RESP) ||
             (p->type < WIFI_VND_IE_TYPE_BEACON)) {
            command_log("Invalid vendor ie type \n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         if ((p->idx > WIFI_VND_IE_ID_1) || (p->idx < WIFI_VND_IE_ID_0)) {
            command_log("Invalid vendor ie ID index \n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         if (!p->vnd_ie.payload) {
            command_log("Invalid vendor IE buffer \n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }
         ctrl_msg__req__set_soft_apvendor_specific_ie__init(req_payload);

         req_payload->enable = p->enable;
         req_payload->type = (CtrlVendorIEType) p->type;
         req_payload->idx = (CtrlVendorIEID) p->idx;

         req_payload->vendor_ie_data = (CtrlMsgReqVendorIEData *)hosted_malloc(sizeof(CtrlMsgReqVendorIEData));

         if (!req_payload->vendor_ie_data) {
            command_log("Mem alloc fail\n");
            break;
         }
         buff_to_free2 = req_payload->vendor_ie_data;

         ctrl_msg__req__vendor_iedata__init(req_payload->vendor_ie_data);

         req_payload->vendor_ie_data->element_id = p->vnd_ie.element_id;
         req_payload->vendor_ie_data->length = p->vnd_ie.length;
         req_payload->vendor_ie_data->vendor_oui.data =p->vnd_ie.vendor_oui;
         req_payload->vendor_ie_data->vendor_oui.len = VENDOR_OUI_BUF;

         req_payload->vendor_ie_data->payload.data = p->vnd_ie.payload;
         req_payload->vendor_ie_data->payload.len = p->vnd_ie.payload_len;
         break;
      } case CTRL_REQ_START_SOFTAP: {
         softap_config_t *p = &app_req->u.wifi_softap_config;
         CTRL_ALLOC_ASSIGN(CtrlMsgReqStartSoftAP, req_start_softap);

         if ((strlen((char *)&p->ssid) > MAX_SSID_LENGTH) ||
             (!strlen((char *)&p->ssid))) {
            command_log("Invalid SSID length\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         if ((strlen((char *)&p->pwd) > MAX_PWD_LENGTH) ||
             ((p->encryption_mode != WIFI_AUTH_OPEN) &&
              (strlen((char *)&p->pwd) < MIN_PWD_LENGTH))) {
            command_log("Invalid password length\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         if ((p->channel < MIN_CHNL_NO) ||
             (p->channel > MAX_CHNL_NO)) {
            command_log("Invalid softap channel\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         if ((p->encryption_mode < WIFI_AUTH_OPEN) ||
             (p->encryption_mode == WIFI_AUTH_WEP) ||
             (p->encryption_mode > WIFI_AUTH_WPA_WPA2_PSK)) {

            command_log("Asked Encryption mode not supported\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         if ((p->max_connections < MIN_CONN_NO) ||
             (p->max_connections > MAX_CONN_NO)) {
            command_log("Invalid maximum connection number\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         if ((p->bandwidth < WIFI_BW_HT20) ||
             (p->bandwidth > WIFI_BW_HT40)) {
            command_log("Invalid bandwidth\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }
         ctrl_msg__req__start_soft_ap__init(req_payload);

         req_payload->ssid = (char *)&p->ssid;
         req_payload->pwd = (char *)&p->pwd;
         req_payload->chnl = p->channel;
         req_payload->sec_prot = (CtrlWifiSecProt)p->encryption_mode;
         req_payload->max_conn = p->max_connections;
         req_payload->ssid_hidden = p->ssid_hidden;
         req_payload->bw = p->bandwidth;
         break;
      } case CTRL_REQ_SET_PS_MODE: {
         wifi_power_save_t * p = &app_req->u.wifi_ps;
         CTRL_ALLOC_ASSIGN(CtrlMsgReqSetMode, req_set_power_save_mode);

         if ((p->ps_mode < WIFI_PS_MIN_MODEM) ||
             (p->ps_mode >= WIFI_PS_INVALID)) {
            command_log("Invalid power save mode\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }
         ctrl_msg__req__set_mode__init(req_payload);

         req_payload->mode = p->ps_mode;
         break;
      } case CTRL_REQ_OTA_WRITE: {
         ota_write_t *p = & app_req->u.ota_write;
         CTRL_ALLOC_ASSIGN(CtrlMsgReqOTAWrite, req_ota_write);

         if (!p->ota_data || (p->ota_data_len == 0)) {
            command_log("Invalid parameter\n");
            failure_status = CTRL_ERR_INCORRECT_ARG;
            break;
         }

         ctrl_msg__req__otawrite__init(req_payload);
         req_payload->ota_data.data = p->ota_data;
         req_payload->ota_data.len = p->ota_data_len;
         break;
      } case CTRL_REQ_SET_WIFI_MAX_TX_POWER: {
         CTRL_ALLOC_ASSIGN(CtrlMsgReqSetWifiMaxTxPower,req_set_wifi_max_tx_power);
         ctrl_msg__req__set_wifi_max_tx_power__init(req_payload);
         req_payload->wifi_max_tx_power = app_req->u.wifi_tx_power.power;
         break;
      } case CTRL_REQ_CONFIG_HEARTBEAT: {
         CTRL_ALLOC_ASSIGN(CtrlMsgReqConfigHeartbeat, req_config_heartbeat);
         ctrl_msg__req__config_heartbeat__init(req_payload);
         req_payload->enable = app_req->u.e_heartbeat.enable;
         req_payload->duration = app_req->u.e_heartbeat.duration;
         if (req_payload->enable) {
            //printf("Enable heartbeat with duration %ld\n", (long int)req_payload->duration);
            //if (CALLBACK_AVAILABLE != is_event_callback_registered(CTRL_EVENT_HEARTBEAT))
               //printf("Note: ** Subscribe heartbeat event to get notification **\n");
         } else {
            //printf("Disable Heartbeat\n");
         }
         break;
      } default: {
         failure_status = CTRL_ERR_UNSUPPORTED_MSG;
         //printf("Unsupported Control Req[%u]",req.msg_id);
         
         break;
      }
   }

   if(failure_status == 0) {
      /* 4. Protobuf msg size */
      tx_protobuf_len = ctrl_msg__get_packed_size(&req);
      if (!tx_protobuf_len) {
         command_log("Invalid tx length\n");
         failure_status = CTRL_ERR_PROTOBUF_ENCODE;
      }
   }

   CMsg msg(tx_protobuf_len);

   if(msg.is_valid() && failure_status == 0) {
      /* 8. Pack in protobuf and send the request */
      ctrl_msg__pack(&req, msg.get_protobuf_ptr());
      //application_send_msg_to_esp32(msg, CTRL_EP_NAME_RESP, ESP_SERIAL_IF, 0);
      //esp_host_notify_spi_driver_to_tx();
   }
   else {
      msg.clear();
      ret = FAILURE;
   }
      
   if(buff_to_free1 != NULL) {
      delete buff_to_free1;
   }
   if(buff_to_free2 != NULL) {
      delete buff_to_free2;
   }
   
   return ret;
}


