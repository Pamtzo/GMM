import sys
import time
import zmq
import numpy as np
import json

def _compute_log_likelihood(j):
     """manually compute the log likelihood of the current parameter"""
     log_likelihood = 0
     for k in range(j["clusters"]):           
         weight = j["weigths"][k]
         mean = j["means"][k]
         cov = j["covs"][k]
         cov_inverse = np.linalg.inv(cov)
         term_other = np.log(2 * np.pi) + np.log(np.linalg.det(cov))
            
         for x in j["data"]:
                # compute (x-mu)^T * Sigma^{-1} * (x-mu)
             diff = x - mean
             term_exponent = np.dot(diff.T, np.dot(cov_inverse, diff))
            
                # compute loglikelihood contribution for this data point and this cluster             
             log_likelihood += -1 / 2 * (term_other + term_exponent) + np.log(weight)
            
     return log_likelihood

def update(j):
    j["means"]=np.array(j["means"])
    j["responsability"]=np.array(j["responsability"])
    j["weigths"]=np.array(j["weigths"])
    j["covs"]=np.array(j["covs"])
    j["data"]=np.array(j["data"])
    return j

def degrade(j):
    j["means"]=j["means"].tolist()
    j["responsability"]=j["responsability"].tolist()
    j["weigths"]=j["weigths"].tolist()
    j["covs"]=j["covs"].tolist()
    j["data"]=j["data"].tolist()
    return j

def fuse(js, key):
    flag=True
    for n in js:
        if flag:
            j=np.array(n[key])
            flag=False
        else:
            j=np.vstack([j[key],n[key]])
    return j

def mstep(j):
    # total responsibility assigned to each cluster, N^{soft}
    resp_weights = j["responsability"].sum(axis = 0)
    
    # weights
    j["weigths"] = resp_weights / j["data"].shape[0]

    weighted_sum = np.dot(j["responsability"].T, j["data"])
    j["means"] = weighted_sum / resp_weights.reshape(-1, 1)

    # covariance
    for k in range(j["clusters"]):
        diff = (j["data"] - j["means"][k]).T
        weighted_sum = np.dot(j["responsability"][:, k] * diff, diff.T)
        j["covs"][k] = weighted_sum / resp_weights[k]
    
    return j

context = zmq.Context()

fan = context.socket(zmq.REQ)
fan.bind("tcp://*:5558")

work = context.socket(zmq.PULL)
work.bind("tcp://*:5559")

workers=0
while True:
    if work.recv_string()=="Ok":
        break
    workers+=1
fan.send_string(str(workers))
fan.recv_string()

fan.send_string("o_challenge")

log_likelihood=0
parada=pow(10,-9)
log_likelihood_trace=[]

# Process 100 confirmations
while True:
    fan.recv_string()
    js=[]
    for i in range(workers):
        answer = work.recv_string()
        j=eval(answer)
        j=update(j)
        js.append(j)
    j=js[0]
    j["responsability"]=fuse(js, "responsability")
    j["data"]=fuse(js, "data")
    j2=mstep(j)
    log_likelihood_new = _compute_log_likelihood(j)
           
    print(j2['means'])
    print(j2['covs'])
    print(j2['weigths'])
    if abs(log_likelihood - log_likelihood_new) <= parada:
        break
    log_likelihood = log_likelihood_new
    log_likelihood_trace.append(log_likelihood)
    j2=degrade(j2)
    fan.send_json(j2)