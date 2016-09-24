#ifndef _BACKGROUNDJOB_H
#define _BACKGROUNDJOB_H

#include <functional>

#include "FOClient.h"

/* A BackgroundJob is a function that gets called every game frame.
 *
 * Every BackgroundJob has a unique name. Starting a job with a name that's already uses stops the previous job.
 */

using BackgroundJob = std::function<void(FOClient*)>;

/* Starts a background job. Removes any previous job with the given name. */
void startJob(std::string name, BackgroundJob job);

/* Stops a job. Does nothing if no job with the given name exists. */
void stopJob(std::string name);

/* Checks if a job with the given name is running. */
bool jobRunning(std::string name);

/* Run the jobs. Should be called from the main loop. */
void runJobs(FOClient*);

#endif //_BACKGROUNDJOB_H
