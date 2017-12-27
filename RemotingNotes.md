# Api Goal

```reason
let system =  start(~seedNodes=["https://discovery1.example.com", "https://discovery2.example.com", "https://discovery3.example.com"]);
```

Possibly the seed nodes run as a separate application? Could possible use a single url with DNS for seed discovery instead of an array?

```reason
type userId =  | UserId(int);
type transaction = { user: userId, amount: string };

let keySelector = ({ user: UserId(id) }) => id;
let shard = clustered(keySelector, "tenant", system);
// Or maybe?
let shard: actorRef('a) = clustered("tenant", Sharded(keySelector), system);
```

## Routing strategies

Sharded(keySelector)

Random

RoundRobin

Affinity

...etc

https://kemptechnologies.com/load-balancer/load-balancing-algorithms-techniques/



```reason
let actor = spawn();
join(shard, actor);
shard +@ actor;
leave(shard, actor);
shard -@ actor; 
```

Dispatching could occur in the normal way

```reason
shard <-< { user: 1234567890; amount: 200 };
```

