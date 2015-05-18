//
// WEBPROC.C - WEB Processing functions
//
// EBS - RTIP
//
// Copyright Peter Van Oudenaren , 1993
// All rights reserved.
// This code may not be redistributed in source or linkable object form
// without the consent of its author.
//
// Module Description:
//   This modeule contains the functions to process HTTP Get commands and
//   HTTP Post commands.
//


#include "sock.h"
#if (INCLUDE_RTIP)
#include "rtip.h"
#include "servers.h"
#endif

#if (INCLUDE_WEB)
#include "web.h"
#include "webapi.h"
#include "vfile.h"

// ********************************************************************
// DEBUG AIDS
// ********************************************************************
#define DISPLAY_FILE 1		// display requested file before escaped
							// chars are fixed
#define DISPLAY_FILE_UNFIXED 0
#define DEBUG_WEB  		0
#define DEBUG_POST 		0
#define DEBUG_MOD_DATE 	0
#define DEBUG_WEB_ROOT_DIR 0
#define DEBUG_PERSIST  0 
#define DEBUG_REALTIME 0

// ********************************************************************
#define FORCE_SEND 0

// By setting this to 1 we check web page for real time data; if this
// is not set, we assume the worst that the file has real time data
#define INCLUDE_CHECK_REALTIME 1

#define NO_REALTIME_DATA "<!-- ** THIS FILE CONTAINS NO REALTIME DATA ** -->"
#define REALTIME_DATA    "<!-- ** THIS FILE DOES CONTAIN REALTIME DATA ** -->"

// ********************************************************************
extern struct server_context KS_FAR http_server_context;
#if (INCLUDE_HTTPS_SRV)
extern struct server_context KS_FAR https_server_context;
#endif
extern char KS_FAR web_start_date[MAX_DATE_LEN];
#if (INCLUDE_WEB_SECURITY)
extern browser KS_FAR browser_ip_table[CFG_MAX_BROWSERS];
#endif		// INCLUDE_WEB_SECURITY
#if (INCLUDE_WEB_AUTHENTICATION)
extern struct web_auth_entry KS_FAR *web_basic_auth;
extern int KS_FAR num_web_basic_auth;
#endif
KS_EXTERN_GLOBAL_CONSTANT char KS_FAR month_strings[NUM_MONTHS][4];
KS_EXTERN_GLOBAL_CONSTANT struct web_file_type KS_FAR web_file_types[WEB_NUM_FILE_TYPES];
#if (INCLUDE_WEB_EXTRA_MIME)
extern PFMIME_FOR_PAGES web_mime_fields;
#endif

// ********************************************************************
int     execute_get_command(PIO_CONTEXT io_context, 
                        struct http_mime_ptrs *mime_ptrs, PFCHAR filename,
                        PFCHAR param);
int     execute_include_function(PIO_CONTEXT io_context, PFCHAR function, PFCHAR param);
int     execute_post_function(PIO_CONTEXT io_context, long len, char *function);
RTIP_BOOLEAN compare_dates(char *date1, char *date2);
PFCHAR  token(PFCHAR zbuffer, PFCHAR *znext, char *ztokens);
PFCHAR  http_read_file_ext(PVFILEAPI p_api,PIO_CONTEXT io_context, PFCHAR buffer, int length, RTIP_BOOLEAN html_flag);
char *  web_get_file_extension(char *filename);
PFBYTE parse_for_include(PFCHAR sptr);
RTIP_BOOLEAN parse_and_process_include(PIO_CONTEXT io_context, PFBYTE buffer, 
                                  PFCHAR param);

// ********************************************************************
// PROCESS GET COMMAND
// ********************************************************************

// checks file for any real time data (i.e. data which might change with
// each download of the page/file; It checks for Server Side Includes (cgi);
// This routine does not check for file name being x.cgi (server push)
// or for virtual files which also may supply realtime data (since this
// routine will not be called for either of these cases)
// ASSUMES: file is open
//
// Returns TRUE if the file does supply realtime data

RTIP_BOOLEAN check_realtime_data(PVFILEAPI p_api, PIO_CONTEXT io_context)
{
PFCHAR  sptr;
RTIP_BOOLEAN ret_val;

    if ((sptr=vf_fgets(p_api, io_context->virtual_fd, io_context->pb_out, 
                            (word)io_context->total_out)) == 0)
		return(FALSE);

	if (tc_strncmp((PFCHAR)io_context->pb_out, REALTIME_DATA, 
				   tc_strlen(REALTIME_DATA)) == 0)
	{
#if (DEBUG_REALTIME)
		DEBUG_ERROR("check_realtime_data: REALTIME DATA", NOVAR, 0, 0);
#endif
		DEBUG_LOG("check_realtime_data: REALTIME DATA", LEVEL_3, NOVAR, 0, 0);
		_vf_rewind(p_api, io_context->virtual_fd);
		return(TRUE);
	}

	if (tc_strncmp((PFCHAR)io_context->pb_out, NO_REALTIME_DATA,
                   tc_strlen(NO_REALTIME_DATA)) == 0)
	{
#if (DEBUG_REALTIME)
		DEBUG_ERROR("check_realtime_data: NO REALTIME DATA", NOVAR, 0, 0);
#endif
		DEBUG_LOG("check_realtime_data: NO REALTIME DATA", LEVEL_3, NOVAR, 0, 0);
		_vf_rewind(p_api, io_context->virtual_fd);
		return(FALSE);
	}

	DEBUG_LOG("check_realtime_data: determine if realtime data", LEVEL_3, NOVAR, 0, 0);
#if (DEBUG_REALTIME)
	DEBUG_ERROR("check_realtime_data: determine if realtime data", NOVAR, 0, 0);
#endif

#if (INCLUDE_CHECK_REALTIME)
    // If it is an html file, then do line oriented IO instead of
	// block IO. This will allow us to easily parse server side
    // includes.
	ret_val = FALSE;
	do
	{
        // Look in the line for an include.
        // If the line has an include, then there is real-time data
        if (parse_for_include(sptr))
		{
			ret_val = TRUE;
			break;
		}
    } while ((sptr=vf_fgets(p_api, io_context->virtual_fd, io_context->pb_out, 
                            (word)io_context->total_out)) != 0);
#else
	ret_val = TRUE;		// assume the worst
#endif

	_vf_rewind(p_api, io_context->virtual_fd);
#if (DEBUG_REALTIME)
	if (ret_val)
	{
		DEBUG_ERROR("check_realtime_data: REALTIME DATA found", NOVAR, 0, 0);
	}
	else
	{
		DEBUG_ERROR("check_realtime_data: NO REALTIME DATA found", NOVAR, 0, 0);
	}

#endif
	return(ret_val);
}

//****************************************************************
// process_get_command() - process GET command received from web client.
//    Processes the GET command as specified by args (i.e. args points
//    to the file to be down loaded.  This routine extracts the file
//    to be down loaded, reads the rest of the request processing
//    any mime header fields (If-Modified-Date is the only one looked
//    at), and calls execute_get_command() to finish the processing.
//
//    Returns -1 if a severe error occurs (such as running out of DCUs),
//    an error code (see enum http_codes) or 0 if successful

int process_get_command(PIO_CONTEXT io_context, 
                        struct http_mime_ptrs *mhdr_in, 
                        PFCHAR args)
{
char filename[CFG_MAX_FILE_LEN+1];
PFCHAR begin;
PFCHAR param;

	param = (PFCHAR)0;

    begin = args;

	// Find the beginning slash.
    while (*begin == ' ') 
		begin++;

    if (*begin == '/') 
	{
    	// Skip the slash
     	begin++;

     	// See if there is a space after the /
     	if (*begin == ' ') 
		{
			// If there was a space, then the filename of interest is
			// index.html
#if (!NACT_OS)
			tc_strcpy(filename, "index.html");
#else
            tc_strcpy(filename, "sysweb/index.htm");
#endif
     	} 
		else 
		{
       		int x = 0;

       		// Copy the filename
       		while ( (begin[x] != ' ') && (begin[x] != 0) && 
			        (begin[x] != '?') && (x < CFG_MAX_FILE_LEN) ) 
			{
	  			filename[x] = begin[x];

			  	#if (INCLUDE_DOS_FS || INCLUDE_ERTFS_API || INCLUDE_RTFILES || INCLUDE_MFS)
	  				if (filename[x] == '/') 
					{
						filename[x] = '\\'; 
					}
	  			#endif

	  			x++;
       		}

			// get parameter to CGI (server side include) or Get form method
			// server side include : extract the query string
			//   from the URL (the query string follows a ?) ;
			//   URL user typed in or from web page, i.e.
			//   <A HREF=http://URL?param=7>
			//   Parameter will be passed to CGI function
			// Get form method : for this case web will receive a
			//   GET in form http://func.cgi?name=PARAM&param=7
			//   This will be sent by the browser when the following
			//   is sent by the server in a webpage after the
			//   browser requests a value for the parameter
			//   <FORM ACTION="http://func.cgi" METHOD="GET">
			//   PARAM<INPUT type="tbd"name="PARAM">
			//   </FORM>
			//   
			if (begin[x] == '?')
				param = &(begin[x+1]);

       		// Make sure it is null terminated.
       		filename[x] = 0;
     	}

		// WE ARE DONE WITH args WHICH POINTS TO io_context->buffer_out

#if (DISPLAY_FILE_UNFIXED)
		DEBUG_ERROR("GET COMMAND: file name before fix is ", 
			STR1, filename, 0);
#endif
		DEBUG_LOG("GET COMMAND: file name before fix is ", 
			LEVEL_3, STR1, filename, 0);

		// some chars are sent in %xx (where xx is hex ascii 
		// representation) format; convert these characters to
		// their ascii representation
		fix_escaped_chars_all(filename);

		DEBUG_LOG("GET COMMAND: file name is ", LEVEL_3, STR1, filename, 0);
		DEBUG_ERROR("GET COMMAND: file name is ", STR1, filename, 0);

#if (INCLUDE_WEB_CWD)
        {
           int l1 = tc_strlen(CFG_WEB_ROOT_DIR);
           int l2 = tc_strlen(filename);
#if (DEBUG_WEB_ROOT_DIR)
		   DEBUG_ERROR("l1, l2 = ", EBS_INT2, l1, l2);
		   DEBUG_ERROR("CFG_WEB_ROOT_DIR: ", STR1, CFG_WEB_ROOT_DIR, 0);
#endif
           if ((l1 + l2) < sizeof(filename))
           {
              memmove(filename + l1, filename, l2+1);
              memmove(filename, CFG_WEB_ROOT_DIR, l1);
           }
        }
#endif

#if (INCLUDE_WEB_AUTHENTICATION)
        /* **************************************************   */
        io_context->auth_mime_header = 0;
        io_context->auth_realm = NULL;
        if (!check_authenticate_request(io_context, filename, mhdr_in))
            return(0);
#endif
		return(execute_get_command(io_context, mhdr_in, filename, param));
   	}

	// file does not start with slash
   	// Return failure. If we got here, it was because the
   	// system had a malformed request. A slash did not exist
   	// After the get command.
 	DEBUG_ERROR("process_get_command - no / in filename", NOVAR, 0, 0);

   	http_send_response(io_context, BAD_REQUEST_400);
   	return(0);
}

RTIP_BOOLEAN open_web_file(char *filename, PIO_CONTEXT io_context, PVFILEAPI *p_api)
{
#if (INCLUDE_MFS)
#if (DEBUG_WEB)
	DEBUG_ERROR("WEB: OPEN MFS FILE", STR1, filename, 0);
#endif
	/* Find it in the memory file system first */
	*p_api = &mfs_api;
    if ((io_context->virtual_fd = _vf_open(*p_api, filename,VO_RDONLY,0)) < 0)
#endif
	{
		/* Not there try the virtual file system */
		*p_api = vf_api;
#if (DEBUG_WEB)
		DEBUG_ERROR("WEB: OPEN VFS FILE", STR1, filename, 0);
#endif
    	if ((io_context->virtual_fd = _vf_open(*p_api, filename,VO_RDONLY,0)) < 0)
		{
			return(FALSE);		// failure
		}
	}
	return(TRUE);				// success
}

// execute_get_command() - execute GET command received from web client.
//    Processes the GET command as specified by filename.
//
//    Returns -1 if a severe error occurs (such as running out of DCUs),
//    an error code (see enum http_codes) or 0 if successful

int execute_get_command(PIO_CONTEXT io_context,
                        struct http_mime_ptrs *mhdr_in,
                        PFCHAR filename, PFCHAR param)
{
#if (INCLUDE_RTIP)
DCU msg, msg_mhdr_out;
#endif
PFBYTE buffer_mhdr_out;
struct http_mime_header KS_FAR *mhdr_out;
RTIP_BOOLEAN real_time;
int     i, offset;
PFBYTE  buffer;
PFBYTE  buffer_sav;
char *  ext;
PFCHAR  sptr;
int     putlen, readsize;
RTIP_BOOLEAN html_flag;
RTIP_BOOLEAN res;
#if (INCLUDE_RTIP)
PTCPPORT port;
#endif
PVFILEAPI p_api;     // Which file system in use currently MFS or Virtual
#if (!IS_MS_PM && !defined(_MSC_VER) && 0)
    // MSDOS does not know the creation date.
    char *cdate;
	char use_buffer[CFG_MAX_FILE_LEN+1];
#endif
	char fdate[MAX_DATE_LEN];
#if (INCLUDE_RTFILES && INCLUDE_VIRTUAL_TABLE)
	char ram_filename[CFG_MAX_FILE_LEN+1];
#endif
#if (0)
char trunc_filename[CFG_MAX_FILE_LEN+1];
#endif
int  chop_len;
int  ext_len;

	////////////////////////////////////////////////////
    ext = web_get_file_extension(filename);

	// .cgi files are resevered for SERVER PUSH requests; the
	// routines to execute for server push requests are in the
	// table in httpgetf.c (in webpages directory)
	if (ext && tc_strnicmp(ext, "cgi", 3) == 0)
	{
    PFCHAR p = param;
    
    if (p)
    {
      while (*p != '\0')
      {
        if (*p == ' ')
          *p = '\0';
        else
          p++;  
      }
    }
    
		execute_include_function(io_context, filename, param);
	}
   	else
	{
		if ((res=open_web_file(filename, io_context, &p_api)) == 0)
		{
			DEBUG_ERROR("execute_get_command - could not open file = ",
				STR1, filename, 0);

			// if extension is greater than 3, try with extenstion of
			// length 3
			ext_len = 0;
			if (ext)
				ext_len = tc_strlen(ext);
			if (ext_len > 3)
#if 0 // this verson consumes a lot of stack space
			{
				tc_strcpy(trunc_filename, filename);
				chop_len = tc_strlen(filename) - ext_len + 3;
				trunc_filename[chop_len] = '\0';
				DEBUG_ERROR("try to open ", STR1, trunc_filename, 0);
				res = open_web_file(trunc_filename, io_context, &p_api);
				DEBUG_ASSERT(res != FALSE, 
							 "execute_get_command - could not open file: ",
							 STR1, trunc_filename, 0);
			}
#else // this one needs less stack space but only works if file name is not needed after the file is opened (which is true)
            {
                chop_len = tc_strlen(filename) - ext_len + 3;
                filename[chop_len] = '\0';
				DEBUG_ERROR("try to open ", STR1, filename, 0);
                res = open_web_file(filename, io_context, &p_api);
                DEBUG_ASSERT(res != FALSE, 
                             "execute_get_command - could not open file: ",
                             STR1, filename, 0);
            }
#endif
			if (res == FALSE)
			{
				http_send_response(io_context, NOT_FOUND_404);
				return(0);
			}
		}

		DEBUG_LOG("WEB: OPENED THE FILE", LEVEL_3, NOVAR, 0, 0);

	    ////////////////////////////////////////////////////
		MALLOC_BUFFER(msg_mhdr_out, buffer_mhdr_out, 
					  sizeof(struct http_mime_header), WEB_MHDR_ALLOC)
		mhdr_out = (struct http_mime_header KS_FAR *)buffer_mhdr_out;

		if (!buffer_mhdr_out)
		{
			DEBUG_ERROR("execute_get_command: os_alloc_packet failed", NOVAR,
				0, 0);

	     	// Close the file.
   			_vf_close(p_api, io_context->virtual_fd);

			return(set_errno(ENOPKTS));
		}

	    ////////////////////////////////////////////////////
		// initialize mime info - i.e. clear all information which
		// will be constructed; this information will be used
		// to send the response back to the browser
		init_mime_header(mhdr_out);  

        /* **************************************************      */
        /* if an additional mime string has been specified then    */
        /* include it in the header.                               */
  		if( io_context->auth_mime_header )
        	HTTP_GET_RETURN_STRING( mhdr_out->return_hdr, 
									io_context->auth_mime_header );
  		mhdr_out->auth_realm = io_context->auth_realm;

        /* **************************************************    */
        /* Support these file types: jpg, htm, gif, mpg, avi etc */
        html_flag = FALSE;

		// find offset in web_file_types for the extention; if it is
		// not in the table use the default (offset 0); if there
		// is no extention assume .htm (offset 1);
		offset = 0;		// default
		if (!ext)	    // use .htm
			offset = 1;
		else
		{
			for (i=1; i < WEB_NUM_FILE_TYPES; i++)
			{
				if (tc_strnicmp(ext, web_file_types[i].ext, 
							    tc_strlen(web_file_types[i].ext)) == 0)
				{
					offset = i;
					break;		// found one
				}
			}
		}

    	tc_strcpy(mhdr_out->content_type, web_file_types[offset].content_type);

		// check for real time data and determine if processing an 
		// html script.
		real_time = FALSE;
		if (offset == 1 || offset == 2)		    // if .htm or .html
		{
		    html_flag = TRUE;
			////////////////////////////////////////////////////
			// check if file contains real time data (CGIs etc)
			// NOTE: must be after open file
			real_time = check_realtime_data(p_api, io_context);
		}

		// if we know the file size, set it up (Content-Length);
		// if there is any real time data (CGIs etc) then we do
		// not know the exact length therefore leave it up to
		// the browser to figure it out
		if (!real_time)
		{
			mhdr_out->clength = vf_filelength(p_api, io_context->virtual_fd);
		}
#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
		else
		{
			// since realtime data, then cannot do persisent connection
			mhdr_in->persistent = FALSE;
			#if (DEBUG_PERSIST)
                DEBUG_ERROR("execute_get_command: no persistent data since real time data"
                    NOVAR, 0, 0);
			#endif
		}
#endif

		////////////////////////////////////////////////////
		// DATE STUFF
#if (!IS_MS_PM && !defined(_MSC_VER) && 0)
    	// MSDOS does not know the creation date.
	    cdate = _vf_get_file_creation_date(p_api, io_context->file_ptr,
                                          use_buffer, 128);
	    if (cdate) 
		{
			tc_strcpy(mhdr_out->cdate, cdate);
	    }
#endif

#if (INCLUDE_WEB_ETAG && 0)
if (mhdr_in->etag)
{
DEBUG_ERROR("return not modified", NOVAR, 0, 0);
http_send_response(io_context, NOT_MODIFIED_304);
FREE_BUFFER(msg_mhdr_out, buffer_mhdr_out);
_vf_close(p_api, io_context->virtual_fd);
return(NOT_MODIFIED_304);
}
#endif

		if (CFG_MOD_DATE)
		{
			// get modified date is used to send date to browser;
			// if browser sent an If-Modified-Since in the request it is
			// also used to determine if the file should be resent or
			// a message indicating the file has not changed should be
			// sent
			// NOTE: the browser will only send an If-Modified-Since
			//       request on subsequent requests for the same page
			//       if the date is returned
			// NOTE: virtual files do not have a modified date and
			//       FALSE will be returned
   			if (_vf_get_file_modified_date(p_api, io_context->virtual_fd, 
									   	(byte *)fdate,
                                       	MAX_DATE_LEN))
	 		{
				#if (DEBUG_MOD_DATE)
					DEBUG_ERROR("execute_get_command: modified date: ",
						STR1, fdate, 0);
				#endif

				// if the file has realtime data then do not return the
				// date and do not check for If-Modified-Since date
	    		if ( (html_flag && !real_time) || !html_flag )
				{
					tc_strcpy(mhdr_out->fdate, fdate);

					// if if-mod-date >= curr date, then return Not-Modified
					// message
					if (mhdr_in->if_mod_date[0] && 
                    	compare_dates(mhdr_in->if_mod_date, fdate))
					{
						DEBUG_LOG("send not mod string", LEVEL_3, NOVAR, 0, 0);
						http_send_response(io_context, NOT_MODIFIED_304);
			     		_vf_close(p_api, io_context->virtual_fd);
						FREE_BUFFER(msg_mhdr_out, buffer_mhdr_out);
						return(NOT_MODIFIED_304);
					}
					DEBUG_LOG("do not send not mod string, ifdate:", 
						LEVEL_3, STR1, mhdr_in->if_mod_date, 0);
					DEBUG_LOG("do not send not mod string, fdate:", 
						LEVEL_3, STR1, fdate, 0);
				}
     		}
			else
			{
				DEBUG_LOG("could not get mod date", LEVEL_3, NOVAR, 0, 0);
			}
		}		// if CFG_MOD_DATE

	    ////////////////////////////////////////////////////
		// SEND THE REQUESTED FILE

#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
		// send persistent mime info based upon if supported by browser
		// and no realtime data
		mhdr_out->persistent = mhdr_in->persistent;
#endif

#if (INCLUDE_WEB_EXTRA_MIME)
		i = 0;
		while (web_mime_fields && web_mime_fields[i].web_page)
		{
			// if there is are mime fields for all pages, pass the
			// first table entry to send_mime_header()
			if (!mhdr_out->web_fields_all_pages && 
                tc_strlen(web_mime_fields[i].web_page) == 0)
			{
				mhdr_out->web_fields_all_pages = web_mime_fields[i].mime_fields;
			}
			if (!mhdr_out->web_fields && 
			    tc_strcmp(web_mime_fields[i].web_page, filename) == 0)
			{
				mhdr_out->web_fields = web_mime_fields[i].mime_fields;
			}
			i++;
		}
#endif

		// first send the MIME header
		send_mime_header(io_context, mhdr_out);

    	// Read the file and send it to the requestor.
    	readsize = io_context->total_out;
    	if (readsize > 1024) 
			readsize = 1024;

    	// To do server side includes, we must watch for the
    	// pattern <!--#exec
    	// When we find this pattern, then look at what follows
    	// the exec command to see what do do with the include.

       	// If it is an html file, then do line oriented IO instead of
		// block IO. This will allow us to easily parse server side
       	// includes.
		res = FALSE;
		
	    ////////////////////////////////////////////////////
		// READ AND SEND THE FILE
	    ////////////////////////////////////////////////////
#if (DEBUG_WEB)
		DEBUG_ERROR("READ AND SEND FILE: ", STR1, filename, 0);
#endif

    	if (!html_flag) 
		{
#if (DEBUG_WEB)
			DEBUG_ERROR("READ AND SEND FILE: NOT HTML", STR1, filename, 0);
#endif
	    ////////////////////////////////////////////////////
		// NON-HTML FILE
	    ////////////////////////////////////////////////////
#if (INCLUDE_RTIP)
			// set up so select will return when bufflen is available
			// (eventually this will be available thru api)
			port = (PTCPPORT)sock_to_port(io_context->sock);
	        if (port)
		        port->select_size = (word)readsize;
#endif
	
			// PROCESS NON .HTML FILE
			// send the mime header since it was queued and we
			// are going to read directly into the DCU to send
			io_context->length_out = 0;
      		xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);

        	while ( (putlen=_vf_read(p_api,io_context->virtual_fd, 
                                    (PFBYTE)io_context->pb_out, 
                                    (word)readsize)) > 0) 
			{
#if (0)
{
char c;
c = *(io_context->pb_out+putlen);
*(io_context->pb_out+putlen) = 0;
DEBUG_ERROR("vf_read: ", STR1, io_context->pb_out, 0);
*(io_context->pb_out+putlen) = c;
}
#endif

            	// Send the buffer back out now.
            	// Note that the NAGLE algorithm will put all of this
            	// data into 1 packet anyway, so calling send with only
            	// 512 bytes per packet will not neccesarily cause
            	// 512 byte packets to be sent out. 
				io_context->offset_out = putlen;
	      		if (xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_SEND) < 0)
				{
					DEBUG_ERROR("execute_get_command: xn_line_put failed",
						NOVAR, 0, 0);
					break;
				}
        	}		// end of while

#if (INCLUDE_RTIP)
	    	port->select_size = 0;
#endif

     	} 

		else
		{
#if (DEBUG_WEB)
			DEBUG_ERROR("READ AND SEND FILE: HTML", STR1, filename, 0);
#endif
		    ////////////////////////////////////////////////////
			// HTML FILE
		    ////////////////////////////////////////////////////
			MALLOC_BUFFER(msg, buffer,  MAX_PACKETSIZE, WEB_XFER_ALLOC);
			if (!buffer)
			{
				DEBUG_ERROR("execute_get_command: DCU alloc failed", NOVAR, 0, 0);
				FREE_BUFFER(msg_mhdr_out, buffer_mhdr_out);
     			_vf_close(p_api, io_context->virtual_fd);
				return(set_errno(ENOPKTS));
			}

	        while ((sptr=http_read_file_ext(p_api, io_context, (PFCHAR)buffer, 
                                            MAX_PACKETSIZE,
 	                                        html_flag)) != 0) 
    	    {
				#if (0)
				DEBUG_ERROR("vf_read: ", STR1, buffer, 0);
				#endif

       	    	/* Look in the line for an include.          */
	            /* If the line has an include, deal with it. */
				io_context->length_out = tc_strlen(sptr);
               	if (html_flag)
                    res = parse_and_process_include(io_context, 
													buffer, param);
	    	    if (!res)
       			{
	                /* Put line out if parse did not deal with it.   */
					buffer_sav = (PFBYTE)io_context->buffer_out;
					io_context->buffer_out = (PFCHAR)buffer;
    		        if (xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE) < 0)
					{
						DEBUG_ERROR("execute_get_command: xn_line_put failed",
							NOVAR, 0, 0);
						break;
					}

#if (FORCE_SEND)
					io_context->length_out = 0;
        		    xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);
#endif

					io_context->buffer_out = (PFCHAR)buffer_sav;
	        	}
            }       /* end of loop reading file */

#if (!FORCE_SEND)
           	/* force the last send since queued data */
			io_context->length_out = 0;
            xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);
#endif

			FREE_BUFFER(msg, buffer)
		}

#if (DEBUG_WEB)
		DEBUG_ERROR("READ AND SEND FILE DONE: ", STR1, filename, 0);
#endif

     	// Close the file.
     	_vf_close(p_api, io_context->virtual_fd);

		FREE_BUFFER(msg_mhdr_out, buffer_mhdr_out);
	}

    // Return success to the caller.
    return(0);
}

char *web_get_file_extension(char *filename) 
{
char *period;

    period = tc_strrchr(filename, (int)'.');

    if (period) 
		return period+1;
	return 0;
}

PFCHAR http_read_file_ext(PVFILEAPI p_api, PIO_CONTEXT io_context, PFCHAR buffer, int bufsize, RTIP_BOOLEAN html_flag)
{
	// If it is an html file, then do line oriented IO instead of
	// block IO. This will allow us to easily parse server side
    // includes.
	if (html_flag)
	{
 		return(vf_fgets(p_api, io_context->virtual_fd, buffer, (word)bufsize));
	}
	else
	{
 		if (_vf_read(p_api, (int)io_context->virtual_fd, (PFBYTE)buffer, (word)bufsize))
			return(buffer);
		else
			return((PFCHAR)0);
	}
}

//****************************************************************
// CGI (SERVER SIDE INCLUDE) - GET
//****************************************************************
// Look in the string for an include. 
//
// Returns 0 if error or did not find an include, address of include if it did
//
PFBYTE parse_for_include(PFCHAR sptr)
{
PFBYTE head;

	// This is the format of an include line.
	//           1111111
	// 01234567890123456              3210
	// <!--#exec cgi="/cgi-bin/counter"-->
	head = (PFBYTE)tc_strstr(sptr, (PFCHAR)"<!--#exec cgi=\"/");

	if (head) 
		return(head);

	// If this function did nothing, return FALSE.
	return((PFBYTE)0);
}

// Look in the string for an include. If it has one, then
// try to deal with it.
// Returns FALSE error or did not find an include, TRUE if it did
#if (0)
RTIP_BOOLEAN parse_and_process_include(PIO_CONTEXT io_context, PFBYTE buffer, 
                                  PFCHAR param)
{
PFCHAR  head, trail;
PFCHAR  ptr;
int     len;
PFBYTE  buffer_save;
RTIP_BOOLEAN found;
char    function[256];

	// get pointer to server side include (if there is one)
	head = parse_for_include((PFCHAR)buffer);

	if (head) 
	{
	    // The line does have a server side include in it.
    	// Now process it.

	    // First send out all data up to the include
	    if (buffer != head) 
		{
			io_context->length_out = (int)(head - buffer);
			buffer_save = io_context->buffer_out;
			io_context->buffer_out = buffer;
			xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);
			io_context->buffer_out = buffer_save;
		}

	    // point head at the function to execute.
	    head += 16;  // Skip the leading stuff (<!--#exec cgi="/)

		found = FALSE;
		ptr = trail = head;

		// find the end of the function to execute
		while (*trail)
		{
			// if at end and no parameter
			if (*trail == '"')
			{
				found = TRUE;
                ptr = trail;
				break;
			}

			// if parameter in webpage
			if (*trail == ' ')
			{
				found = TRUE;

				// extract parameter for server side include
				// NOTE: if parameter passed in on URL use that parameter first
				if (!param)
				{
					param = trail + 1;

					// find last " and replace with eos
					if ((ptr = tc_strchr(param, '"')) != 0)
						*ptr = '\0';
				}
				break;
			}
			trail++;
		}		

	    // If there is no trailing > then bail.
    	if (!found)
		{
    	   DEBUG_ERROR("Error: include format missing trailing \"",
			     NOVAR, 0, 0);
	       return FALSE;
    	}

    	// Now get the function name.
	    len = (int)((dword)trail - (dword)head);
    	if (len > 255) 
			len = 255;

	    tc_strncpy((PFCHAR)function, head, len);
    	function[len] = 0;

        DEBUG_LOG("Found Server side include", LEVEL_3, NOVAR, 0, 0);
        DEBUG_LOG(function, LEVEL_3, NOVAR, 0, 0);

	    // Now that we have the function name, go to the function
    	// parse table and see if we can execute this function.
    	execute_include_function(io_context, function, param);

	    // Put out anything trailing the server side include
    	if (tc_strlen(ptr+4)) 
		{
			tc_strcpy(io_context->buffer_out, ptr+4);
			io_context->length_out = tc_strlen(io_context->buffer_out);
			xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);
		}

		return(TRUE);
	}

	// If this function did nothing, return FALSE.
	return(FALSE);
}
#else
RTIP_BOOLEAN parse_and_process_include(PIO_CONTEXT io_context, PFBYTE buffer, 
                                  PFCHAR param)
{
PFBYTE  head, trail;
PFBYTE  ptr;
int     len;
PFBYTE  buffer_save;
RTIP_BOOLEAN found;
char    function[256];

    /* get pointer to server side include (if there is one)   */
    while( (head = parse_for_include((PFCHAR)buffer))!=NULL )
   	{
        /* The line does have a server side include in it.   */
        /* Now process it.                                   */

        /* First send out all data up to the include   */
        if (buffer != head) 
        {
            io_context->length_out = (int)(head - buffer);
            buffer_save = (PFBYTE)io_context->buffer_out;
            io_context->buffer_out = (PFCHAR)buffer;
            xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);
            io_context->buffer_out = (PFCHAR)buffer_save;
        }

        /* point head at the function to execute.   */
        head += 16;  /* Skip the leading stuff (<!--#exec cgi="/) */

        found = FALSE;
        ptr = trail = head;

        /* find the end of the function to execute   */
        while (*trail)
        {
            /* if at end and no parameter   */
            if (*trail == '"')
            {
                found = TRUE;
                ptr = trail;
                break;
            }

            /* if parameter in webpage   */
            if (*trail == ' ')
            {
                found = TRUE;

                /* extract parameter for server side include                    */
                /* NOTE: if parameter passed in on URL use that parameter first */
                if (!param)
                {
                    param = (PFCHAR)(trail + 1);

                    /* find last " and replace with eos   */
                    if ((ptr = (PFBYTE)tc_strchr((PFCHAR)param, '"')) != 0)
                        *ptr = '\0';
                }
                break;
            }
            trail++;
        }

        /* If there is no trailing > then bail.   */
        if (!found)
        {
           DEBUG_ERROR("Error: include format missing trailing \"",
                 NOVAR, 0, 0);
           return FALSE;
        }

        /* Now get the function name.   */
        len = (int)((dword)trail - (dword)head);
        if (len > 255)
            len = 255;

        tc_strncpy((PFCHAR)function, (PFCHAR)head, len);
        function[len] = 0;

        DEBUG_LOG("Found Server side include", LEVEL_3, NOVAR, 0, 0);
        DEBUG_LOG(function, LEVEL_3, NOVAR, 0, 0);

        /* Now that we have the function name, go to the function   */
        /* parse table and see if we can execute this function.     */
        execute_include_function(io_context, function, param);

        // reset the parameter pointer, we only want to use the ones
        // passed into this function once.
        param = NULL;
        // skip past the '"-->' at the end of a CGI call.
        buffer = ptr+4;
    }

    /* Put out anything trailing the server side include   */
	io_context->length_out = tc_strlen((PFCHAR)buffer);
	if( io_context->length_out )
	{
		tc_strcpy( (PFCHAR)io_context->buffer_out, (PFCHAR)buffer );
		xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);
	}
    /* If this function did nothing, return FALSE.   */
	return TRUE;
}
#endif

//****************************************************************
// Return non-zero if the function was executed.
int execute_include_function(PIO_CONTEXT io_context, PFCHAR function, 
                             PFCHAR param)
{
int x = 0;

    // The end of the table is marked by a null function.
    while (get_function_table[x].function_ptr) 
	{
       if (tc_strcmp((PFCHAR)function, get_function_table[x].function_name) == 0) 
	   {
         // If the function exists, execute it.
         get_function_table[x].function_ptr(io_context, param);
         return 1;
       }
       // Look at the next entry in the table.
       x++;
    }

    return 0;
}

// ********************************************************************
// PROCESS POST COMMAND
// ********************************************************************

//****************************************************************
// PROCESS POST COMMAND received from WEB client.
// The remainder of the POST command will be in args.
// If the requested POST function is executed, then
// the function will return 0. Otherwise non-zero.
//
int process_post_command(PIO_CONTEXT io_context, struct http_mime_ptrs *mime_ptrs, PFCHAR args)
{
long len;
char command[128];
PFCHAR spc;
int ret_val;

#if (DEBUG_POST)
	DEBUG_ERROR("process_post_command called", NOVAR, 0, 0);
#endif

    // Find the command (which is the name of the function to
	// execute) and null terminate it.
    spc = args+1;
    while ((*spc != ' ') && (*spc != 0)) 
		spc++;
    if (*spc == ' ') 
		*spc = 0;

    tc_strncpy((PFCHAR)command, args+1, 127);
    command[127] = 0;

	// WE ARE DONE WITH args WHICH POINTS TO io_context->buffer_out

	len = mime_ptrs->clength;
    ret_val = execute_post_function(io_context, len, command);

    /* force the last send since queued data */
	io_context->length_out = 0;
    xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);

	return(ret_val);
}

//****************************************************************
// Return 0 if the function was executed.
int execute_post_function(PIO_CONTEXT io_context, long len, char *function)
{
int x = 0;

    while (post_function_table[x].function_ptr) 
	{
		DEBUG_LOG("execute_post_function: function, table function = ",
			LEVEL_2, STR2, function, post_function_table[x].function_name);
    	if (tc_strcmp((PFCHAR)function, post_function_table[x].function_name) == 0) 
	   	{
#if (DEBUG_POST)
			DEBUG_ERROR("execute_post_command: call POST fnc", NOVAR, 0, 0);
#endif
        	post_function_table[x].function_ptr(io_context, len);
         	return 0;
       	}
       	x++;
    }
    http_send_response(io_context, NOT_FOUND_404);

    return 1;
}

// ********************************************************************
// PARSER
// ********************************************************************

RTIP_BOOLEAN skip_token(PFCHAR p, int *offset, char *ztokens)
{
RTIP_BOOLEAN p_is_separator;
char *t;

	p_is_separator = FALSE;
	while (*(p + *offset) != '\0' && !p_is_separator)
	{                                    
   		t = ztokens;                      
   		while (*t != '\0' && *p != *t) 
			t++;
									  
	   	if (*t != '\0') 
			p_is_separator = TRUE;
   		else 
			(*offset)++;                             
	}
	return(p_is_separator);
}

//****************************************************************
// get_command - parse zline for token specified in cmds structure
//
//     parse zline for tokens in cmds structure (POST or GET) where
//     cmds is the list of commands to search for
//
//     returns number associated with the command found (i.e. GET_CMD or
//     POST_CMD) or INVALID_COMMAND if command is not recognized 
//
//     zline is the command line to be parsed
//
//     zargs will return a pointer to the first argument following the command found
//

long get_command(PFCHAR zline, PFCHAR *zargs)
{
int i = 0;
PFCHAR ztoken;

   	ztoken = token(zline, zargs, " \t\n\r");

	if (ztoken) 
   	{
    	while (web_cmds[i].lnumber != INVALID_CMD && 
		       tc_stricmp(ztoken, web_cmds[i].zcommand)) 	
	  	{
			i++;
      	}
   	} 
	else 
		return(INVALID_CMD);

	return(web_cmds[i].lnumber);
}

//****************************************************************
// get_version - parse buffer for HTTP version
//
//     Parses buffer for tokens HTTP/ to extract the major and minor version
//     numbers.  If the version string does not exist, version 0.9
//     is returned (as specified in section 3.1 of RFC 1945).
//
//     buffer is the command line to be parsed which has format
//     HTTP/x.y  (x is major number and y is minor number)
//
//     returns number associated with the command found (i.e. GET_CMD or
//     POST_CMD) or INVALID_COMMAND if command is not recognized 
//
//

void get_version(PFCHAR buffer, int bufsize, int *major_number, int *minor_number)
{
int maj, min;
int i;
int tok_len;

	// set to default (version 0.9)
	*major_number = 0;
	*minor_number = 9;

	i = 0;

	// search the string for the token "HTTP/"; NOTE: continue
	// searching past any eos until end of buffer
	tok_len = tc_strlen((PFCHAR)"HTTP/");
	while (i < bufsize)
	{
		if (!tc_strncmp(buffer+i, "HTTP/", tok_len))
		{
			i += tok_len;
			maj = tc_atoi(buffer+i);
			while (buffer[i] != '.')
			{
				if (buffer[i] < '0' || buffer[i] > '9')	 
					return;
				i++;
			}
			i++;		// skip the period
			min = tc_atoi(buffer+i);
			*major_number = maj;
			*minor_number = min;
			return;
		}
		i++;
	}
}

//****************************************************************
// PARSE MIME HEADER
//****************************************************************

//****************************************************************
// parse_mime_one - parse buffer for mime information such as If-Modified-Since
//
//     Parses buffer for tokens If-Modified-Since; if it is found parse_mime_one
//     returns a pointer to the date after the token. 
//
//     buffer is the command line to be parsed
//
//     returns pointer to date in buffer or 0 if the token is not found
//

void parse_mime_one(struct http_mime_ptrs *mhdr_in, PFCHAR buffer, int bufsize)		/*__fn__*/
{
#if (INCLUDE_CHECK_REALTIME)
int ims_tok_len;
#endif
int auth_tok_len;
PFCHAR znext, znext1, ztoken;
PFCHAR cptr;
int i;

	znext = (PFCHAR)0;

	// search the string for the token If-Modified-Since:
	// searching past any eos until end of buffer
#if (INCLUDE_CHECK_REALTIME)
	ims_tok_len = tc_strlen((PFCHAR)"If-Modified-Since:");
#endif
	auth_tok_len = tc_strlen((PFCHAR)"Authorization: Basic");
	i = 0;

	while (i < bufsize)
	{
#if (INCLUDE_CHECK_REALTIME)
		///////////////////////////////////////////////////
		if (!tc_strncmp(buffer+i, "If-Modified-Since:", ims_tok_len))
		{
			// skip the token
		   	ztoken = token(buffer+i, &znext, " \t\n\r");
			if (ztoken)
			{
				// copy off the date since might overwrite the buffer when
				// read new data
				tc_strncpy(mhdr_in->if_mod_date, znext, MAX_DATE_LEN);
			}
		}
		else
#endif
		///////////////////////////////////////////////////
		if (!tc_strncmp(buffer+i, "Authorization: Basic", auth_tok_len))
		{
			// skip the token Authorization:
		   	ztoken = token(buffer+i, &znext, " \t\n\r");
			if (ztoken)
			{
				// skip the token Basic
			   	ztoken = token(znext, &znext1, " \t\n\r");
				if (ztoken)
				{
					// find length of auth_basic_cookie
					mhdr_in->auth_basic_cookie_len = 0;
					cptr = znext1;
					while (cptr < buffer+bufsize)
					{
						if (*cptr == '\r' || *cptr == '\n')
							break;
						mhdr_in->auth_basic_cookie_len++;
						cptr++;
					}
	
					// check if length is larger than configured for
					if (mhdr_in->auth_basic_cookie_len > CFG_MAX_AUTH_COOKIE)
					{
						DEBUG_ERROR("parse_mime_one: CFG_MAX_AUTH_COOKIE is not large enough: need, act = ",
							EBS_INT2, mhdr_in->auth_basic_cookie_len,
							CFG_MAX_AUTH_COOKIE);

						// try to continue although will fail in check
						mhdr_in->auth_basic_cookie_len = 
							CFG_MAX_AUTH_COOKIE;
					}
	
					// copy the encoded string
					// NOTE: need to copy instead of saving a pointer
					//       since xn_line_get keep reading into the
					//       buffer which could write over the info
					tc_movebytes(mhdr_in->auth_basic_cookie, znext1,
						mhdr_in->auth_basic_cookie_len);
				}
			}
		}

		///////////////////////////////////////////////////
#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
		// header field names are case-insensitive (section 3.4.7 of RFC 822)
		if (!tc_strnicmp(buffer+i, "Connection: close", 
					     tc_strlen("Connection: close")))
		{
DEBUG_ERROR("connection: close => no persistent", NOVAR, 0, 0);
			mhdr_in->persistent = FALSE;
		}

		// header field names are case-insensitive (section 3.4.7 of RFC 822)
		if (!tc_strnicmp(buffer+i, "Connection: Keep-Alive", 
					     tc_strlen("Connection: Keep-Alive")))
		{
			mhdr_in->keep_alive = TRUE;
		}
#endif

		///////////////////////////////////////////////////
#if (INCLUDE_WEB_ETAG)
		// tbd
		if (tc_strnicmp(buffer+i, "If-None-Match:", 
			 	        tc_strlen("If-None-Match:")) == 0)
		{
DEBUG_ERROR("etag found", NOVAR, 0, 0);
			mhdr_in->etag = buffer+i+tc_strlen("If-None-Match: ");
		}
#endif
		///////////////////////////////////////////////////
        if (tc_strnicmp(buffer+i, (PFCHAR)"Content-length:", 15) == 0)	 
		{
           mhdr_in->clength = tc_atol(buffer+i+15);
        }

		i++;
	}

	return;
}

void parse_mime_header(PIO_CONTEXT io_context, struct http_mime_ptrs *mhdr_in)
{
PFCHAR buffer_in;
int    buf_len;

	tc_memset(mhdr_in, 0, sizeof(http_mime_ptrs));

#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
	mhdr_in->keep_alive = FALSE;

	// if not browser is version 1.1 or later, it would not support 
	// persistent connections by default
	if ( (io_context->major_number < 1) ||
	     (io_context->minor_number < 1) )
	{
//DEBUG_ERROR("major, minor => no persistent", EBS_INT2,
//	io_context->major_number, io_context->minor_number);
		mhdr_in->persistent = FALSE;
	}
	else
		mhdr_in->persistent = TRUE;
#endif

	////////////////////////////////////////////////////
   	// Read the rest of the request from the client.
   	while ((buf_len = xn_line_get(io_context, &buffer_in, CFG_WEB_TIMEOUT, 
                                  GET_LINE)) > 0) 
	{
		// parse the line of the mime header, for example
		// get If-Modified-Date from request if it exists
		parse_mime_one(mhdr_in, buffer_in, buf_len);
	}

#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
	// if the browser does not support keep-alive, then server will
	// not support persistent connection (tbd)
	// NOTE: it will also be set to false if there is realtime data
	if (!mhdr_in->keep_alive)
	{
DEBUG_ERROR("parse_mime_header: no keepalive => no persistent", NOVAR, 0, 0);
		mhdr_in->persistent = FALSE;
	}
#endif

}


//****************************************************************
#define SKIP_SEPARATOR                                                  \
while (*p != '\0' && p_is_separator) 									\
{                                     									\
   t = ztokens;                                                         \
   while (*t != '\0' && *p != *t)		 								\
      t++;                                  							\
																		\
   if (*t == '\0') 														\
      p_is_separator = FALSE;                                           \
   else																	\
      p++;                                                              \
}

#define SKIP_TOKEN                                                      \
while (*p != '\0' && !p_is_separator) 									\
{                                    									\
   t = ztokens;                                                         \
   while (*t != '\0' && *p != *t) t++;                                  \
																		\
   if (*t != '\0') p_is_separator = TRUE;                               \
   else p++;                                                            \
																		\
}

//****************************************************************
// search string zbuffer for token which ends in one of characters
// in ztokens;
// ztokens is a list of separator characters;
// *znext is set to pointer after the token
PFCHAR token(PFCHAR zbuffer, PFCHAR *znext, char *ztokens)
{
PFCHAR p;
char *t;
PFCHAR token = 0;
RTIP_BOOLEAN p_is_separator = TRUE;

    p = zbuffer;

    // skip past all the separator characters in ztokens at the beginning 
	// of zbuffer;
    // p_is_separator is set to FALSE if a non-separator command encountered
	// before hit the end of the string - p points to the non-separator
	// character
    SKIP_SEPARATOR

	// if a non-separator was found
	if (!p_is_separator) 
    {
    	/* p now points to the first token in the string */
      	token = p;

		// skip over the next token stopping at one of the separator characters
		// specified in ztokens
      	SKIP_TOKEN

		// if found a separator after the token
     	if (p_is_separator) 
	  	{
			// put eos char after the token and point p one past the eos
			if (*p != '\0') 
	 		{
	    		*p = '\0';
	    		p++;
	 		}

			// skip past any separator characters
	 		SKIP_SEPARATOR

			// return pointer to token after the command
	 		if (znext) 
				*znext = p;

      	} 

		// if no separator after the token then return 0 for pointer
		// to string after command
		else if (znext) 
			*znext = 0;
   }

   return(token);

}

//****************************************************************
// FIX INPUT STRING
//****************************************************************
// replace all occurances in buffer of the 3 chars in ns_string with
// the character in replace_char
// NOTE: this ecsaping can be done in fields fill out in form and in the
//       URI name
void fix_escaped_chars(PFCHAR buffer, char *ns_string, char replace_char)
{
PFCHAR ns_ptr;
int str_len;

	str_len = tc_strlen((PFCHAR)ns_string);

	// browser can send %2C for commas, etc so fix it (i.e. replace ns_string
	// with replace char)
	while ( (ns_ptr = tc_strstr(buffer, ns_string)) != 0 )
	{
		ns_ptr[0] = replace_char;
		tc_strcpy(ns_ptr+1, ns_ptr+str_len);
	}
}

// replace all occurances in buffer of the 3 chars %xx with the correct
// ascii character
// NOTE: this ecsaping can be done in fields fill out in form and in the
//       URI name
void fix_escaped_chars_all(PFCHAR buffer)
{
PFCHAR ns_ptr;
char buf[3];
char replace_char;

	// browser can send %2C for commas, etc so fix it (i.e. replace %2C with
	// , etc.
	while ( (ns_ptr = tc_strstr(buffer, "%")) != 0 )
	{
		buf[0] = *(ns_ptr+1);
		buf[1] = *(ns_ptr+2);
		buf[2] = '\0';

		// if it is not a number then could be the % separating fields
		if (tc_isnum(buf))
		{
			replace_char = (char)tc_hatoi(buf);
			ns_ptr[0] = replace_char;
			tc_strcpy(ns_ptr+1, ns_ptr+3);	
		}
		buffer = ns_ptr+1;		// make sure don't search from beginning
								// or if %25 exists it will get changed 
								// to % which will match on the next search
	}
}

//****************************************************************
// OUTPUT MIME HEADER
//****************************************************************

//****************************************************************
// Send the mime header
//
//   Sends the mime headers whose field values have been set up in
//   the structure hdr to the remote host.
//
//   io_context contains the socket to send the header to.  The
//   buffer field of io_context is not modified.
//
//   Returns 0 if success; -1 if error
//

int send_mime_header(PIO_CONTEXT io_context, 
                     struct http_mime_header KS_FAR *hdr)
{
byte buffer[MAX_MIME_HEADER_SIZE];
char string[16];
int  ret_val;
PFBYTE buffer_sav;

//	int i=16;
//  tbd - add error checking for writing past the end of buffer

	buffer_sav = (PFBYTE)io_context->buffer_out;
	io_context->buffer_out = (PFCHAR)buffer;

#if (1)
	//    Changed io_context->to buffer -- seems to be Borland compiler bug. 
	//    Not pushing actual segment by re-reading the field. Was pushing dx, 
	//    which contains old segment (of old buffer_out) before it is changed 
	//    on line above.
    tc_strcpy((PFCHAR)buffer, hdr->return_hdr);
#else
	tc_strcpy((PFCHAR)io_context->buffer_out, hdr->return_hdr);
#endif

#if (INCLUDE_WEB_11 && INCLUDE_WEB_PERSIST_CONNECT)
	if (hdr->persistent == FALSE)
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\nConnection: Close");
	else
	{
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\nConnection: Keep-Alive");

		tc_strcat((PFCHAR)io_context->buffer_out, "\r\nKeep-Alive: timeout = ");
		tc_itoa(CFG_WEB_KEEPALIVE_TMO, (PFCHAR)(
                io_context->buffer_out+tc_strlen((PFCHAR)io_context->buffer_out)), 
                10);
		tc_strcat((PFCHAR)io_context->buffer_out, ", max = ");
		tc_itoa(CFG_WEB_KEEPALIVE_MAX, (PFCHAR)(
                io_context->buffer_out+tc_strlen((PFCHAR)io_context->buffer_out)), 
                10);
	}

#endif

	if (tc_strlen(hdr->cdate))
	{
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\nFile-created: ");
		tc_strcat((PFCHAR)io_context->buffer_out, hdr->cdate);
	}

	tc_strcat((PFCHAR)io_context->buffer_out, "\r\nServer: Micro-Web");
	
	if (tc_strlen(hdr->content_type))
	{
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\nContent-type: ");
		tc_strcat((PFCHAR)io_context->buffer_out, hdr->content_type);
	}
	if (tc_strlen(hdr->fdate))
	{
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\nLast-modified: ");
		tc_strcat((PFCHAR)io_context->buffer_out, hdr->fdate);
	}
	if ((hdr->clength) > -1)
	{
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\nContent-length: ");
		tc_ltoa(hdr->clength, string, 10);
		tc_strcat((PFCHAR)io_context->buffer_out, string);
	}
#if (INCLUDE_WEB_AUTHENTICATION)
    if (hdr->auth_realm)
	{
		tc_strcat((PFCHAR)io_context->buffer_out, 
				  "\r\nWWW-Authenticate: Basic realm=\"");
		tc_strcat((PFCHAR)io_context->buffer_out, hdr->auth_realm);
		tc_strcat((PFCHAR)io_context->buffer_out, "\"");
	}
#endif

#if (INCLUDE_WEB_EXTRA_MIME)
	if (hdr->web_fields_all_pages)
	{
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\n");
		tc_strcat((PFCHAR)io_context->buffer_out, hdr->web_fields_all_pages);
	}
	if (hdr->web_fields)
	{
		tc_strcat((PFCHAR)io_context->buffer_out, "\r\n");
		tc_strcat((PFCHAR)io_context->buffer_out, hdr->web_fields);
	}
#endif

	// terminate the mime header by an empty mime field, i.e. "\r\n"
	tc_strcat((PFCHAR)io_context->buffer_out, "\r\n\r\n");

	// send the mime header to the browser
	io_context->length_out = tc_strlen((PFCHAR)io_context->buffer_out);
	ret_val = xn_line_put(io_context, CFG_WEB_TIMEOUT, PUT_QUE);
	io_context->buffer_out = (PFCHAR)buffer_sav;
	return(ret_val);
}

// Initializes the mime header structure; used for sending mime headers
void init_mime_header(struct http_mime_header KS_FAR *hdr)
{
	if (hdr)
	{
		tc_memset(hdr, 0, sizeof(struct http_mime_header));
		hdr->clength = -1;
	}

	// get the string to return with the correct version number
	// NOTE: assumes OK header is send
	HTTP_GET_RETURN_STRING(hdr->return_hdr, OK_200); 
}

#if (0)
// read to the end of the mime header; i.e read until a blank line is
// encountered as per RFC 822 section 3.1 (the mime header is separated
// from the headers by a null line)
// extracts Content-length if found
void read_mime_header(PIO_CONTEXT io_context, long wait, long *len)
{
PFCHAR buffer;

	*len = 0;
	// once hit a 0 length line we are at the end of the mime header
	// since mime headers end in 2 newlines
    while (xn_line_get(io_context, &buffer, wait, GET_LINE) > 0)
	{
		//DEBUG_ERROR("read_mime_header: buffer:", STR1, buffer, 0);
        //                              0123456789012345
        if (tc_strnicmp(buffer, (PFCHAR)"Content-length:", 15) == 0)	 
		{
           *len = tc_atol(buffer+15);
        }
    }
}
#endif

//****************************************************************
// UTILITIES
//****************************************************************

//****************************************************************
RTIP_BOOLEAN tc_isnum(PFCHAR s)
{
	// skip over tabs and spaces
	while ( (*s == ' ') || (*s == '\t') )
		s++;

	if (*s == '-')
		s++;

	if (*s && (*s >= '0' && *s <= '9'))
		return(TRUE);
	else
		return(FALSE);
}

//****************************************************************
// Return 1 (true) if the character is 0, \n, &
RTIP_BOOLEAN check_char(PFCHAR buffer) 
{
	if ( (*(buffer) == '&') || (*(buffer) == '\n') || (*(buffer) == '\0') ) 
   		return TRUE;
   	else 
   		return FALSE;
}

//****************************************************************
// TIME UTILITIES
//****************************************************************
int extract_next(PFCHAR *zargs)
{
int value;
PFCHAR ztoken;

	value = tc_atoi(*zargs);
	ztoken = token(*zargs, zargs, " ,-:");
	if (!ztoken)
		return(-1);
	return(value);
}

RTIP_BOOLEAN extract_date_info(char *date, int *month, int *day, int *year, 
                          int *hour, int *minute, int *second)
{
PFCHAR ztoken, zargs;
int i;

   	ztoken = token(date, &zargs, " ,");	// skip the day of the week
	if (!ztoken)
		return(FALSE);

	*day = extract_next(&zargs);
	if ( (*day < 0) || !zargs)
		return(FALSE);

	*month = -1;
	for (i=0; i<NUM_MONTHS; i++)
	{
		if ( !tc_strnicmp(&month_strings[i][0], zargs, 
		                  tc_strlen(&month_strings[i][0])) )
		{
			*month = (i+1);
			break;
		}
	}

	token(zargs, &zargs, " ,-");	// skip the month
	if ( (*month == -1) || !zargs)
		return(FALSE);

	*year = extract_next(&zargs);
	if ( (*year < 0) || !zargs)
		return(FALSE);

	// some years include the thousands and some don't, i.e.
	// could be 96 or 1996 so make 96 1996 so can compare dates
	// and adjust for year 2000 problem (i.e. make 00-70 into 2000)
	// NOTE: this assumes files were not created before year 1970
	if (*year >= 70 && *year < 100)
		*year = *year + 1900;
	else if (*year < 70)
		*year = *year + 2000;

	*hour = extract_next(&zargs);
	if ( (*hour < 0) || !zargs)
		return(FALSE);

	*minute = extract_next(&zargs);
	if ( (*minute < 0) || !zargs)
		return(FALSE);
   	
	*second = extract_next(&zargs);
	if (*second < 0)
		return(FALSE);
	return(TRUE);
}

// returns TRUE if if date 1 is >= data2; or FALSE if data1 < date2 or
// dates could not be extracted
RTIP_BOOLEAN compare_dates(char *date1, char *date2)
{
int month1, day1, year1;
int hour1, minute1, second1;
int month2, day2, year2;
int hour2, minute2, second2;
int tot_sec1, tot_sec2;

	if (!extract_date_info(date1, &month1, &day1, &year1, &hour1, &minute1, 
	                       &second1))
		return(FALSE);
	if (!extract_date_info(date2, &month2, &day2, &year2, &hour2, &minute2, 
	                       &second2))
		return(FALSE);


	if (year1 > year2)
		return(TRUE);
	else if (year1 < year2)
		return(FALSE);

	if (month1 > month2)
		return(TRUE);
	else if (month1 < month2)
		return(FALSE);

	if (day1 > day2)
		return(TRUE);
	else if (day1 < day2)
		return(FALSE);


	tot_sec1 = hour1 + 60*minute1 + 60*second1;
	tot_sec2 = hour2 + 60*minute2 + 60*second2;

	if (tot_sec1 >= tot_sec2)
		return(TRUE);

	return(FALSE);  	
}


#if (INCLUDE_WEB_PUT)
//****************************************************************
// PROCESS PUT COMMAND received from WEB client. (publishing fn)
// The filename will be in args.
int process_put_command(PIO_CONTEXT io_context, PFCHAR args)
{
int    n_recv;
PFCHAR pbuffer;	/* used to optimize writing to disk */
char   filename[128];
PFCHAR spc;

    // Find the filename and null terminate it.
    spc = args+1;
    while ((*spc != ' ') && (*spc != 0)) 
		spc++;
    if (*spc == ' ') 
		*spc = 0;

    tc_strncpy((PFCHAR)filename, args+1, 127);
    filename[127] = 0;

#if (INCLUDE_WEB_CWD)
    {
       int l1 = strlen(CFG_WEB_ROOT_DIR);
       int l2 = strlen(filename);
       if ((l1 + l2) < sizeof(filename))
       {
          memmove(filename + l1, filename, l2+1);
          memmove(filename, CFG_WEB_ROOT_DIR, l1);
       }
    }
#endif

	if ((io_context->virtual_fd = 
         vf_open(filename,VO_WRONLY|VO_CREAT|VO_TRUNC,(VS_IWRITE | VS_IREAD))) < 0)
	{
		DEBUG_ERROR("Could not open file for writing", STR1, filename, 0);
		return -1;
	}

	// read the file from the network and store it in a file (or
	// virtual file)
	// NOTE: this would be faster if based the loop on the file size
	//       from the mime header instead of waiting until the
	//       read times out - tbd
    pbuffer = (PFCHAR)io_context->pb_in;
   	while ((n_recv=xn_line_get(io_context, &pbuffer, CFG_WEB_TIMEOUT, 
                               GET_BUF)) > 0) 
	{
		if (n_recv > 0)
		{	
			// write to file
			vf_write((int)io_context->virtual_fd, (PFBYTE) pbuffer, (word) n_recv);
		}
		else
			break;
	}

	http_send_response(io_context, OK_PUT_200);

	vf_close(io_context->virtual_fd);

	return 0;
}
#endif // INCLUDE_WEB_PUT
#endif // INCLUDE_WEB


