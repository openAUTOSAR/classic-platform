
#ifndef COM_INTERNAL_H_
#define COM_INTERNAL_H_


/*-----------------------------------------------------------------*/

/*
 * The only information about the COM that is valid is
 * in the COM specification ..SWS_COM.pdf.
 *
 * The most important requirements are COM010 and COM013
 *
 * Com_Init()
 * Com_DeInit()
 *
 * No error hooks..
 * No. GetMessageStatus()
 * No. SendZeroMessage()
 * No. SendDynamicMessage(), RecieveDynamicMessage()
 * Yes. SendMessage()
 *
 * */

typedef enum OsMessageProperty {
	// ???
	SEND_STATIC_INTERNAL,
	// messages are not consumed during read
	RECEIVE_UNQUEUED_INTERNAL,
	// We have an internal queue
	RECEIVE_QUEUE_INTERNAL,
} OsMessagePropertyType;



typedef enum OsMessageNotificationAction {
	MESSAGE_NOTIFICATION_ACTION_NONE=0,
	MESSAGE_NOTIFICATION_ACTION_ACTIVATETASK,
	MESSAGE_NOTIFICATION_ACTION_SETEVENT,
} OsMessageNotificationActionType;

typedef struct OsMessageNotification {
	OsMessageNotificationActionType type;
	TaskType 		task_id;
	EventMaskType 	event_id;
} OsMessageNotificationType;


typedef struct OsMessage {
	OsMessagePropertyType		property;		// send/recieve...
	OsMessageNotificationType 	notification;
	int 						q_size; 		// 0-Not queued

	// TODO: This is not a good solution but it will have to do for now
	void *data;
	int data_size;
} OsMessageType;


#endif /* COM_INTERNAL_H_ */
