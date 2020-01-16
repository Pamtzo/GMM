import zmq
import pandas as pd
import random
import numpy as np
#from sklearn.preprocessing import StandardScaler

def covs(clusters, dimension, dimension2):
    c=[]
    for n in range(clusters):
        c.append([])
        for m in range(dimension):
            c[n].append([])
            for l in range(dimension2):
                c[n][m].append(random.randint(0,100))
    return c

#scaler = StandardScaler()
n_clusters=2
data=pd.read_csv('dataset.txt')
n_datos=len(data.index)

context = zmq.Context()
#scaler.fit(data.values)
#s=scaler.transform(data.values)
#data=pd.DataFrame(s)

# socket with workers
workers = context.socket(zmq.PUSH)
workers.bind("tcp://*:5557")

# socket with sink
sink = context.socket(zmq.REP)
sink.connect("tcp://localhost:5558")

sink2 = context.socket(zmq.PUSH)
sink2.connect("tcp://localhost:5559")

print("Press enter when workers are ready...")
_ = input()
print("sending tasks to workers")

sink2.send_string('Ok')
n_workers=int(sink.recv_string())#workers
print("somos %i trabajadores" % n_workers)
sink.send_string("Ok")

task=int(n_datos/n_workers)

likelihood=0
weights=[]
means=[]
log_likelihood_trace = []
for i in range(n_clusters):
    means.append(data.loc[random.randint(0,len(data.index)-1)].values.tolist())
    weights.append(1/n_clusters)
    
shape=n_clusters, len(data.columns), len(data.columns)
covs = covs(shape[0], shape[1], shape[2])
sink.recv_string()
print(means)
print(covs)
print(weights)

while True:
    x=0
    sink.send_string('Ok')
    print("The number of registers is %s" % task)
    for w in range(n_workers):
        challenge={
            'data':data.loc[x:x+task].values.tolist(),
            'covs':covs,
            'weights':weights,
            'means':means,
            'dimension':data.shape[1],
            'clusters':n_clusters,
            'size':len(data.loc[x:x+task].index)
        }
        x+=task
        print(challenge,type(challenge))
        workers.send_json(challenge)
    new=sink.recv_json()
    covs=new['covs']
    weights=new['weigths']
    means=new['means']