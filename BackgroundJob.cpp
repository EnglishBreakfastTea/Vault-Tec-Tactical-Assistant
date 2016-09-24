#include <map>
#include <cstdio>

#include "BackgroundJob.h"

std::map<std::string, BackgroundJob> jobs;

void startJob(std::string name, BackgroundJob job)
{
    jobs[name] = job;
}

void stopJob(std::string name)
{
    jobs.erase(name);
}

bool jobRunning(std::string name)
{
    return jobs.find(name) != std::end(jobs);
}

void runJobs(FOClient* client)
{
    for (auto job: jobs) {
        job.second(client);
    }
}
