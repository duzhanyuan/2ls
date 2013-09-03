/*******************************************************************\

Module: Do a jobs for a repository

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>

#include <util/prefix.h>
#include <util/tempfile.h>

#include "job_status.h"
#include "do_job.h"

/*******************************************************************\

Function: init

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void init(job_statust &job_status)
{
  std::string command;
  temporary_filet tempfile("deltagit", "txt");

  // do a git show to learn more about the job
  command="cd source-repo; git show "+job_status.commit+
          " --numstat"+
          " > "+tempfile();

  int result1=system(command.c_str());
  if(result1!=0)
  {
    job_status.failure=true;
    // don't write, commit might be bogus
    return;
  }

  // parse the file
  std::ifstream in(tempfile().c_str());
  if(!in) return;
  
  std::string line;
  
  job_status.added=0;
  job_status.deleted=0;
  job_status.message="";
  
  while(std::getline(in, line))
  {
    if(has_prefix(line, "    git-svn-id: "))
    {
    }
    else if(has_prefix(line, "    "))
    {
      // commit message
      job_status.message+=line.substr(4, std::string::npos)+"\n";      
    }
    else if(has_prefix(line, "Author: "))
    {
      job_status.author=line.substr(8, std::string::npos);
    }
    else if(!line.empty() && isdigit(line[0]))
    {
      // <num-added>\t<num-deleted>\t<file-name>
      const std::size_t end_added=line.find('\t', 0);
      if(end_added==std::string::npos) continue;
      const std::size_t end_deleted=line.find('\t', end_added+1);
      if(end_deleted==std::string::npos) continue;
      
      job_status.added+=atol(line.substr(0, end_added).c_str());
      job_status.deleted+=atol(line.substr(end_added+1, end_deleted-end_added-1).c_str());
    }
  }       
  
  // strip trailing \n from commit message
  std::string &message=job_status.message;
  while(!message.empty() && message[message.size()-1]=='\n')
    message.resize(message.size()-1);
  
  job_status.status=job_statust::BUILD;
  job_status.write();  
}

/*******************************************************************\

Function: check_out

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void check_out(job_statust &job_status)
{
  std::string working_dir=job_status.id+".wd";

  std::string command;

  // do a shared clone -- this uses very little disc space
  command="git clone --no-checkout --shared source-repo "+
          working_dir;

  int result1=system(command.c_str());
  if(result1!=0)
  {
    job_status.failure=true;
    job_status.write();
    return;
  }
  
  // now do checkout; this will eat disc space
  command="cd "+working_dir+"; "+
          "git checkout --detach "+job_status.commit;
  int result2=system(command.c_str());

  if(result2!=0)
  {
    job_status.failure=true;
    job_status.write();
    return;
  }

  job_status.status=job_statust::BUILD;
  job_status.write();  
}

/*******************************************************************\

Function: build

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void build(job_statust &job_status)
{
  job_status.failure=true;
}

/*******************************************************************\

Function: analyse

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void analyse(job_statust &job_status)
{
  job_status.failure=true;
}

/*******************************************************************\

Function: do_job

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void do_job(job_statust &job_status)
{
  while(job_status.status!=job_statust::DONE &&
        !job_status.failure)
  {
    switch(job_status.status)
    {
    case job_statust::INIT: init(job_status); break;
    case job_statust::CHECK_OUT: check_out(job_status); break;
    case job_statust::BUILD: build(job_status); break;
    case job_statust::ANALYSE: analyse(job_status); break;
    case job_statust::DONE: break;
    }
  }

}

/*******************************************************************\

Function: do_job

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void do_job(const std::string &id)
{
  // get current job status
  job_statust job_status(id);
  do_job(job_status);
}

/*******************************************************************\

Function: do_job

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void do_job()
{
  // get job list
  std::list<job_statust> jobs;
  get_jobs(jobs);
  
  // do jobs that need work
  for(std::list<job_statust>::iterator
      j_it=jobs.begin();
      j_it!=jobs.end();
      j_it++)
  {
    if(j_it->status!=job_statust::DONE &&
       !j_it->failure)
    {
      std::cout << "Job " << j_it->id << std::endl;
      do_job(*j_it);
    }
  }
}
