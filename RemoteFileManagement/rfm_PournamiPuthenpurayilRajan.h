/*
 * rfm_PournamiPuthenpurayilRajan.h
 *
 *  Created on: Oct 3, 2018
 *      Author: pournami
 */

#ifndef RFM_POURNAMIPUTHENPURAYILRAJAN_H_
#define RFM_POURNAMIPUTHENPURAYILRAJAN_H_

#define OP_SIZE			13312
#define CMD_LEN			32
#define PATH_LEN		64
#define STATUS_LEN		32

/**
 * request - structure to store the request message
 * command: to store the command portion of the request
 * path: to store the path to file if any
 */
struct request
{
	char command[CMD_LEN];
	char path[PATH_LEN];
};

/**
 * response - to store the response message
 * status_code: to store the status code 200(SUCCESS), 400(BAD REQUEST), 500(SERVER INTERNAL ERROR)
 * status: to store the status according to the status code
 * output: to store the response message
 */
struct response
{
	int status_code;
	char status[STATUS_LEN];
	char output[OP_SIZE];
};

#endif /* RFM_POURNAMIPUTHENPURAYILRAJAN_H_ */
