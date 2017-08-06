

int set_msg_frame(int index, struct comm_message *msg, int size, char *frame);

int remove_first_nframe(int nframe, struct comm_message *msg);

int get_max_msg_frame(struct comm_message *msg);


#endif	/* ifndef _COMM_MESSAGE_OPERATOR_H_ */

