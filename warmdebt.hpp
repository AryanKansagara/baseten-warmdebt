#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <sstream>
namespace warm {
struct Stages{int download,load,warmup;int total()const{return download+load+warmup;}};
struct Request{int arrival,service,budget;};
enum class Policy{reactive,mean_schedule,margin_schedule};
inline std::string name(Policy p){switch(p){case Policy::reactive:return "reactive";case Policy::mean_schedule:return "mean_schedule";case Policy::margin_schedule:return "margin_schedule";}throw std::runtime_error("unknown policy");}
struct Replica{int launch,download_done,load_done,ready,available;};
struct Execution{int arrival,start,finish,deadline;std::size_t replica;};
struct Result{std::string policy;std::vector<Replica> replicas;std::vector<Execution> requests;int misses=0,replica_ticks=0,busy_ticks=0,ready_idle_ticks=0;};
inline void valid_stage(Stages s){if(s.download<0||s.load<0||s.warmup<0||s.download>10000||s.load>10000||s.warmup>10000)throw std::runtime_error("invalid readiness stage");}
inline int lead(Policy p,const std::vector<int>&training){if(p==Policy::reactive)return 0;if(training.size()>10000)throw std::runtime_error("training exceeds bound");if(training.empty())throw std::runtime_error("abstain: no training readiness observations");for(int t:training)if(t<0||t>30000)throw std::runtime_error("invalid training observation");if(p==Policy::margin_schedule)return *std::max_element(training.begin(),training.end());if(p==Policy::mean_schedule)return std::accumulate(training.begin(),training.end(),0LL)/static_cast<long long>(training.size());throw std::runtime_error("unknown policy");}
inline Result simulate(const std::vector<Request>&trace,Policy policy,const std::vector<int>&training,int predicted,Stages stages,int target,int horizon){
 valid_stage(stages);name(policy);if(target<1||target>32||trace.size()>10000||horizon<0||horizon>100000||predicted<0||predicted>horizon)throw std::runtime_error("invalid simulation bounds");
 for(std::size_t i=0;i<trace.size();++i){auto q=trace[i];if(q.arrival<0||q.arrival>horizon||q.service<1||q.service>1000||q.budget<0||q.budget>10000||(i&&q.arrival<trace[i-1].arrival))throw std::runtime_error("invalid or unsorted trace");}
 Result out;out.policy=name(policy);auto margin=lead(policy,training);
 if(policy==Policy::reactive&&trace.empty())return out;
 int start=policy==Policy::reactive?trace.front().arrival:predicted-margin;
 for(int i=0;i<target;++i){int d=start+stages.download,l=d+stages.load,r=l+stages.warmup;out.replicas.push_back({start,d,l,r,r});out.replica_ticks+=std::max(0,horizon-start);out.ready_idle_ticks+=std::max(0,horizon-std::max(0,r));}
 for(auto q:trace){std::size_t choice=0;int begin=std::max(q.arrival,out.replicas[0].available);for(std::size_t i=1;i<out.replicas.size();++i){int possible=std::max(q.arrival,out.replicas[i].available);if(possible<begin){begin=possible;choice=i;}}int finish=begin+q.service;out.replicas[choice].available=finish;out.requests.push_back({q.arrival,begin,finish,q.arrival+q.budget,choice});out.misses+=finish>q.arrival+q.budget;int occupied=std::max(0,std::min(horizon,finish)-std::max(0,begin));out.busy_ticks+=occupied;out.ready_idle_ticks-=occupied;}
 return out;
}
inline std::string json(const Result&r){std::ostringstream s;s<<"{\"policy\":\""<<r.policy<<"\",\"deadline_misses\":"<<r.misses<<",\"replica_ticks\":"<<r.replica_ticks<<",\"busy_ticks\":"<<r.busy_ticks<<",\"ready_idle_ticks\":"<<r.ready_idle_ticks<<",\"readiness_stages\":[";for(std::size_t i=0;i<r.replicas.size();++i){if(i)s<<',';auto x=r.replicas[i];s<<"{\"launch\":"<<x.launch<<",\"download_done\":"<<x.download_done<<",\"load_done\":"<<x.load_done<<",\"ready\":"<<x.ready<<"}";}s<<"],\"requests\":[";for(std::size_t i=0;i<r.requests.size();++i){if(i)s<<',';auto x=r.requests[i];s<<"{\"arrival\":"<<x.arrival<<",\"start\":"<<x.start<<",\"finish\":"<<x.finish<<",\"deadline\":"<<x.deadline<<",\"replica\":"<<x.replica<<"}";}return s.str()+"]}";}
inline std::string demo(){std::vector<Request>trace(4,{100,2,4});std::string s="{\"kind\":\"deterministic_synthetic_simulation\",\"provider_called\":false,\"training_startup_ticks\":[6,9,12],\"heldout_stage_ticks\":[3,5,4],\"same_arrival_trace\":true,\"policies\":[";bool first=true;for(auto p:{Policy::reactive,Policy::mean_schedule,Policy::margin_schedule}){if(!first)s+=",";first=false;s+=json(simulate(trace,p,{6,9,12},100,{3,5,4},2,130));}return s+"]}";}
}
