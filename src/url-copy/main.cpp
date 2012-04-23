#include <iostream>
#include <ctype.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <transfer/gfal_transfer.h>
#include "common/logger.h"
#include "common/error.h"
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include "mq_manager.h"
#include <fstream>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "parse_url.h"
#include <uuid/uuid.h>

using namespace FTS3_COMMON_NAMESPACE;
using namespace std;
namespace po = boost::program_options;

std::string generateLogFileName(std::string surl, std::string durl){
   std::string new_name = std::string("");
   char *base_scheme = NULL;
   char *base_host = NULL;
   char *base_path = NULL;
   int base_port = 0;
   std::string shost;
   std::string dhost; 
   
	uuid_t id;
	char cc_str[36];

	uuid_generate(id);;
	uuid_unparse(id, cc_str);
	string uuid = cc_str;    
   
    //add surl / durl
    //source
    parse_url(surl.c_str(), &base_scheme, &base_host, &base_port, &base_path);
    shost = base_host;
    
    //dest
    parse_url(durl.c_str(), &base_scheme, &base_host, &base_port, &base_path); 
    dhost = base_host;

    // add date
    time_t current;
    time(&current);
    struct tm * date = gmtime(&current);

    // Create template
    std::stringstream ss;
    ss << std::setfill('0');
    ss << std::setw(4) << (date->tm_year+1900)
       <<  "-" << std::setw(2) << (date->tm_mon+1)
       <<  "-" << std::setw(2) << (date->tm_mday)
       <<  "-" << std::setw(2) << (date->tm_hour)
               << std::setw(2) << (date->tm_min)
    << "__" << shost << "__" << dhost << "__" << uuid;

    new_name += ss.str();
    return new_name;
}

class logger {
public:

    logger( std::ostream& os_ );

    template<class T>
    friend logger& operator<<( logger& log, const T& output );

private:

    std::ostream& os;
};

logger::logger( std::ostream& os_) : os(os_) {}

template<class T>
logger& operator<<( logger& log, const T& output ) {
    log.os << output << std::endl;
    log.os.flush(); 
    return log;
}

int main(int argc, char **argv) {

    std::string job_id;
    std::string file_id; // (generated on server, passed to fts3_url_copy, NAME OF THE TR LOG FILE)	
    std::string source_url;
    std::string dest_url;
    //int overwrite; 
    int nbstreams;
    int tcpbuffersize;
    int blocksize;
    int timeout;
    //int daemonize;
    std::string checksum;
    std::string checksumAlgorithm;
    
    po::options_description desc("fts3_url_copy options");
    desc.add_options()
            ("help,h", "help")
            ("job_id", po::value<string > (&job_id), "job id")
            ("file_id", po::value<string > (&file_id), "file id")
            ("source_url", po::value<string > (&source_url), "source url")
            ("dest_url", po::value<string > (&dest_url), "dest url")
            ("overwrite", "overwrite")
            ("nbstreams", po::value<int>(&nbstreams)->default_value(5), "nbstreams")
            ("tcpbuffersize", po::value<int>(&tcpbuffersize)->default_value(0), "tcpbuffersize")
            ("blocksize", po::value<int>(&blocksize)->default_value(0), "blocksize")
            ("timeout", po::value<int>(&timeout)->default_value(3600), "timeout")
            ("daemonize", "daemonize")
            ("console", "console")
            ("blocking", "blocking")
            ;

    po::variables_map vm;
    // parse regular options
    po::positional_options_description p;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    


    // parse positional options
    po::store(po::command_line_parser(argc, argv). options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    if (argc == 1) {
	FTS3_COMMON_LOGGER_NEWLOG (ERR) << " Source url and Destination url are required" << commit;
        return EXIT_FAILURE;
    }

    if (vm.count("job_id")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Job id: " << vm["job_id"].as<string > ()  << commit;	
    }

    if (vm.count("file_id")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "File id: " << vm["file_id"].as<string > ()  << commit;		
    }

    if (vm.count("source_url")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) <<  "Source url: " << vm["source_url"].as<string > ()  << commit;			
    } else {
        FTS3_COMMON_LOGGER_NEWLOG (ERR) << " Source url is required to be set" << commit;
        return EXIT_FAILURE;
    }

    if (vm.count("dest_url")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Destination url: " << vm["dest_url"].as<string > ()  << commit;				
    } else {
        FTS3_COMMON_LOGGER_NEWLOG (ERR) << " Destination url is required to be set" << commit;
        return EXIT_FAILURE;
    }

    std::string surl =  vm["source_url"].as<string > ();
    std::string durl =  vm["dest_url"].as<string > ();
    std::string logFileName = "/var/log/fts3/";
    logFileName += generateLogFileName(surl, durl);
    std::ofstream logStream;
    logStream.open (logFileName.c_str(), ios::app);    
    logger log(logStream);    
    
    log << "Job id: " << vm["job_id"].as<string > ();
    log << "File id: " << vm["file_id"].as<string > ();
    log << "Source url: " << vm["source_url"].as<string > ();
    log << "Destination url: " << vm["dest_url"].as<string > ();

    if (vm.count("nbstreams")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "nbstreams was set to " << vm["nbstreams"].as<int>() << commit;
        log << "nbstreams is set to " << vm["nbstreams"].as<int>();
    }

    if (vm.count("tcpbuffersize")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "tcpbuffersize was set to " << vm["tcpbuffersize"].as<int>() << commit;	
    }

	log << "tcpbuffersize was set to " << vm["tcpbuffersize"].as<int>();

    if (vm.count("blocksize")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "blocksize was set to " << vm["blocksize"].as<int>() << commit;		
	
    }

	log << "blocksize was set to " << vm["blocksize"].as<int>();

    if (vm.count("timeout")) {
	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Timeout was set to " << vm["timeout"].as<int>() << commit;		    
    }

	log << "Timeout was set to " << vm["timeout"].as<int>();

    if (vm.count("overwrite")){
    	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Overwrite was set" << commit;
	log << "Overwrite was set";		           
	}

    if (vm.count("console")){
    	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Print console was set" << commit;	
	log << "Print console was set";	               
	}

    if (vm.count("blocking")){
    	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Blocking mode was set" << commit;		                           
	log << "Blocking mode was set";
	}

    if (vm.count("daemonize")) {
    	FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Daemonize was set" << commit;		                               
	log << "Daemonize was set";
        int value = daemon(0, 0);
	if(value != 0)
	    	FTS3_COMMON_LOGGER_NEWLOG (ERR) << "Failed to set daemon, will continue atatched to the controlling terminal " << commit;		                               	
    }

    FTS3_COMMON_LOGGER_NEWLOG (INFO) <<  " Transfer Accepted" << commit;
    
    log << "Transfer accepted";

    log << "Setup the reported queue back to fts3_server for this process";
    //init message_queue
    QueueManager* qm = NULL;
    try {
        qm = new QueueManager(job_id, file_id);
    } catch (interprocess_exception &ex) {
        if (qm)
            delete qm;
        std::cout << ex.what() << std::endl;
    }

    //start transfer gfal2	
    GError * tmp_err = NULL; // classical GError/glib error management
    gfal_context_t handle;
    int ret = -1;
  
    log << "initialize gfal2";   
    if ((handle = gfal_context_new(&tmp_err)) == NULL) {
	FTS3_COMMON_LOGGER_NEWLOG (ERR) << "Transfer Initialization failed - errno: " <<  tmp_err->message << " Error message:" << tmp_err->message << commit;
	log << "Transfer Initialization failed - errno: " <<  tmp_err->message << " Error message:" << tmp_err->message;
        return -1;
    }
    log << "begin copy";
        FTS3_COMMON_LOGGER_NEWLOG (INFO) <<  " Transfer Started" << commit;
    if ((ret = gfalt_copy_file(handle, NULL, source_url.c_str(), dest_url.c_str(), &tmp_err)) != 0) {        
	FTS3_COMMON_LOGGER_NEWLOG (ERR) << "Transfer failed - errno: " <<  tmp_err->message << " Error message:" << tmp_err->message << commit;
	log << "Transfer failed - errno: " <<  tmp_err->message << " Error message:" << tmp_err->message;
        return -1;
    } else{
        FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Transfer completed successfully"  << commit;
	log << "Transfer completed successfully";
    }
    
    log << "release all gfal2 resources";
    gfal_context_free(handle);
    if (qm)
        delete qm;

    return EXIT_SUCCESS;
}

