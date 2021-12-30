import numpy as np

def gen_dataset(numofsamples=500, w1=3, w2=5, w3=10, w4=20, b=1):
    # y = b + w1 * x1 + w2 * (x2 ^ 2) + w3 * (x3 ^ 3) + w4 * (x4 ^ 4)
    
    np.random.seed(42)
    X = np.random.rand(numofsamples, 4)    
    
    X[:,1] = X[:,1]**2
    X[:,2] = X[:,2]**3
    X[:,3] = X[:,3]**4

    coef = np.array([w1, w2, w3, w4])
    bias = b
    
    #print(X) 
    #print(X.shape)
    
    #print(coef)
    #print(coef.shape)
    
    y = np.matmul(X, coef.transpose()) + bias
    
    #X= (numofsamples, 4), coef.T = (4, 1)
    
    #print(y)
    #print(y.shape)
    
    return X, y

if '__name__' == "'__main__'":
    X, y = gen_dataset()
    print("============================생성한 입력값 데이터 ===========================")
    print(X)
    print(X.shape)
    print()
    print("============================생성한 출력값 데이터 ===========================")
    print(y)
    print(y.shape)