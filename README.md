# WarmDebt

A C++17 deterministic readiness/queue laboratory comparing capacity cost with request deadlines. Baseten serves model workloads and already offers autoscaling schedules. The experimental question is how much readiness margin a particular workload needs, not whether scheduled prewarming can be invented.

## Run

```sh
make test
make demo
./build/warmdebt replay trace.txt
```

Requires GNU Make and C++17 only. CLI replay accepts sorted whitespace-separated `arrival service latency_budget` integer triples, one request per line. It uses the explicitly fixed fixture configuration: training startup observations `[6,9,12]`, predicted burst 100, held-out download/load/warmup `[3,5,4]`, two replicas, horizon 1000. Demo uses horizon 130. No units are silently interpreted as seconds or dollars; all timestamps/costs are abstract ticks. Caller-provided configurations are available through `warmdebt.hpp`.

## Implemented model

All policies consume the **identical** held-out arrival/service/budget trace and actual readiness stages. Training observations determine only launch lead; evaluation readiness is kept separate. Each replica passes through download, load and warmup before becoming available. Requests are FIFO, assigned to the replica offering the earliest start, with deterministic index tie-breaking. A replica serves one request at a time. Service is non-preemptive. Deadline equality succeeds.

- Reactive starts two replicas at the first arrival; with no arrivals it starts none.
- Mean schedule starts at predicted burst minus integer mean training startup.
- Margin schedule starts at predicted burst minus maximum training startup.

Replicas stay until the evaluation horizon. Replica-ticks count launch-to-horizon provisioning, including pre-zero warmup; busy and ready-idle ticks cover the observation window `[0,horizon]`. Requests can finish beyond the horizon: their known deterministic deadline outcome is still counted, while resource accounting is clipped. JSON includes per-replica stage timestamps and per-request start/finish/deadline evidence. Missing training observations abstain for schedule policies.

## Actual evidence

**10 groups / 114 assertions pass**. The designed burst has four requests at tick 100, service two, budget four. Reactive misses four deadlines at 60 replica-ticks; mean schedule misses four at 78; margin misses zero at 84. This is a constructed tradeoff, not a general claim that one policy wins. `demo.json` records actual simulator output.

Negative controls show that reactive avoids schedule cost with no traffic, all policies succeed for one request with zero startup, surprise early arrivals defeat the schedule, and unseen longer startup defeats the trained margin. A hand-calculated single-replica FIFO case checks starts, equality deadlines, idle and busy accounting. Invalid traces/stages/capacities reject; repeat runs are identical. CI is supplied but unexecuted remotely.

## Sources and limits

- [Baseten autoscaling schedules](https://www.baseten.co/blog/autoscaling-schedules-reduce-cold-start-delays-for-predictable-traffic/)
- [Concurrent requests](https://www.baseten.co/resources/changelog/monitor-concurrent-inference-requests/)
- [Baseten inference stack](https://www.baseten.co/resources/guide/the-baseten-inference-stack/)

This is a bounded deterministic simulator, not an inference engine or an implementation of Baseten's production autoscaler. It has no scale-down, dynamic reactive backlog scaling, heterogeneous replicas, network delays, stochastic fitting, prediction intervals or calibration guarantee. The maximum observed training startup is not a statistical confidence bound. Input dimensions/counts are bounded; traces over 10,000 requests and training samples over 10,000 reject. No provider requests, provisioning, observed Baseten trace, cost in currency, live deadline improvement or deployment has been validated. Next: a versioned observed-trace importer and held-out calibration before live shadow recommendations.
