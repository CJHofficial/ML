from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Input, Dropout, Dense
from tensorflow.keras import initializers
from tensorflow import keras

def gen_sequential_model(X, y, EPOCHS = 200, VAL_SPLIT = 0.3):
    #모델 설계
    model = Sequential([
            Input(4, name='input_layer'),
            Dense(8, activation='relu', name='hidden_layer1', kernel_initializer=initializers.RandomNormal(mean=0.0, stddev=0.05, seed=42)),
            #Dropout(1e-9),
            Dense(4, activation='relu', name='hidden_layer2', kernel_initializer=initializers.RandomNormal(mean=0.0, stddev=0.05, seed=42)),
            #Dense(2, activation='relu', name='hidden_layer3', kernel_initializer=initializers.RandomNormal(mean=0.0, stddev=0.05, seed=42)),
            Dense(1, activation='relu', name='output_layer', kernel_initializer=initializers.RandomNormal(mean=0.0, stddev=0.05, seed=42))
            ])
            
    model.summary()
    #print(model.layers[0].get_weights())
    #print(model.layers[1].get_weights())
    
    ADAM = keras.optimizers.Adam(learning_rate=5e-5, beta_1=0.9, beta_2=0.999, amsgrad=False, epsilon=None)
    model.compile(optimizer=ADAM, loss='mse')
    #model.compile(optimizer='sgd', loss='mse') #sgd => stocastic gradient descent, mse => mean sqaure of error

    #모델 학습
    history = model.fit(X, y, epochs=EPOCHS, verbose=2, validation_split= VAL_SPLIT)  
    
    return model, history

def predict_new_sample(model, x, w1=3, w2=5, w3=10, w4=20, b=1):
    
    x = x.reshape(1,4)
    
    x[0][1] = ( x[0][1]**2 )
    x[0][2] = ( x[0][2]**3 )
    x[0][3] = ( x[0][3]**4 )
    
    y_pred = model.predict(x)[0][0]
    
    y_actual = b + w1 * x[0][0] + w2 * x[0][1] + w3 * x[0][2] + w4 * x[0][3]
    
    print("y actual value = ", y_actual)
    print("y predicted value = ", y_pred)