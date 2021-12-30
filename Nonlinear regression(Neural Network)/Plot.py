import matplotlib.pyplot as plt

def print_Loss(myhistory):
    plt.figure(figsize=(15, 10))

    plt.plot(myhistory.history['loss'][1:])
    plt.plot(myhistory.history['val_loss'][1:])
    plt.title('model loss')
    plt.ylabel('loss')
    plt.xlabel('epoch')
    plt.legend(['train', 'test'], loc='upper right')
    plt.show()   
    
    print("train loss=", myhistory.history['loss'][-1])
    print("test loss=", myhistory.history['val_loss'][-1])
