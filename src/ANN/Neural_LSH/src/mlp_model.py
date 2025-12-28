import torch
import numpy as np

import torch.nn as nn
import torch.nn.functional as F


# define the architecture

class MLP_net(nn.Module):
    def __init__(self, input_size=28*28, output_size=10, num_layers=3, hidden_layer=512, dropout_p=0.2):
        super(MLP_net, self).__init__()

        # input images' dimension
        self.input_size = input_size
        # output: number of labels
        self.output_size = output_size
        self.num_layers = num_layers

        self.fc = nn.ModuleList()

        # fully-connected layer 1 : input --> hidden_1
        self.fc.append(nn.Linear(self.input_size, hidden_layer))

        # fully-connected layer 2 : hidden_1 --> hidden_2
        for i in range(self.num_layers-2):
            self.fc.append(nn.Linear(hidden_layer, hidden_layer))

        # fully-connected layer 3 : hidden_2 --> output
        self.fc.append(nn.Linear(hidden_layer, self.output_size))

        # apply dropout to avoid overfitting
        self.dropout = nn.Dropout(dropout_p)

    def forward(self, x):
        # flatten image so it can proceed as a 1xinput_size tensor
        x = x.view(-1, self.input_size)

        for i in range(self.num_layers-1):
            # pass the fc layer and apply reLu
            x = self.fc[i](x)
            x = F.relu(x)
            # dropout layer
            x = self.dropout(x)
    
        # pass the last fc layer without activation function and dropout
        x = self.fc[-1](x)

        # return the result without activation function, softmax activation will be applied in next steps
        return x        
    

#################################################################
# Steps for training:                                           #
# 1. Forward pass to compute the predicted outcome              #
# 2. Calculate the loss                                         #
# 3. Backpropagation                                            #
# 4. Update the parameters with an optimizer step               #
# 6. Update average loss                                        #
# 7. When all the dataset finish go to next epoch               #
#################################################################

def training(model, loss_function, optimizer,train_loader, valid_loader, epochs, data_type, device):
    
    ####### combination of two optimizers ##########
    # ADAM: faster drop better to start with
    optimizer_adam = optimizer

    # SGD: consistent drop for a number of epochs
    optimizer_sgd = torch.optim.SGD(
        model.parameters(),
        lr=0.01,
        weight_decay = 1e-4,
        momentum=0.9
    )

    epoch_change = 40 

    # initialize tracker for minimum validation loss
    valid_loss_min = 100000.0 # set initial "min" to infinity

    # number of epochs without improvement
    no_improve = 0

    for epoch in range(epochs):
        # check train and valid loss
        train_loss = 0
        valid_loss = 0

        # choose optimizer depending on the epoch
        if epoch < epoch_change:
            optimizer = optimizer_adam
        else:
            optimizer = optimizer_sgd

        ######## train the model ########
        model.train()  # set the model in training mode
        for data, target in train_loader:
            data, target = data.to(device), target.to(device)
            # clear the gradients
            optimizer.zero_grad()

            # forward pass and compute the outputs for each batch
            output = model(data)

            # calculate loss
            loss = loss_function(output, target)

            # backpropagation
            loss.backward()

            # optimizer step to update the weights and bias(parameters)
            optimizer.step()

            # update training loss
            train_loss += loss.item()

        
        ######## evaluate the model ###########
        model.eval() # set the model to evaluation mode

        with torch.no_grad():
            for data, target in valid_loader:
                data, target = data.to(device), target.to(device)
                # forward pass and compute the outputs for each batch
                output = model(data)
                # calculate the loss
                loss = loss_function(output, target)
                # update validation loss 
                valid_loss += loss.item()

        # print training/validation statistics 
        # calculate average loss over an epoch
        train_loss = train_loss/len(train_loader)
        valid_loss = valid_loss/len(valid_loader)
        
        print('Epoch: {} \nTraining Loss: {:.6f} \tValidation Loss: {:.6f}'.format(
            epoch+1, 
            train_loss,
            valid_loss
            ))
        
        # save model if validation loss has decreased
        if valid_loss <= valid_loss_min:
            print('Validation loss decreased ({:.6f} --> {:.6f}).  Saving model ...\n'.format(
            valid_loss_min,
            valid_loss))
            torch.save(model, f'model/model_{data_type}.pth')
            
            valid_loss_min = valid_loss
            no_improve = 0
        else:
            no_improve += 1
            if no_improve > 10:
                break


###################################################
########### Testing the model #####################
###################################################
def testing(model, test_loader, loss_function, batch_size, device):
    # initialize lists to monitor test loss and accuracy
    test_loss = 0.0
    class_correct = list(0. for i in range(10))
    class_total = list(0. for i in range(10))

    model.eval() # prep model for evaluation

    for data, target in test_loader:
        data, target = data.to(device), target.to(device)
        # forward pass: compute predicted outputs by passing inputs to the model
        output = model(data)
        # calculate the loss
        loss = loss_function(output, target)
        # update test loss 
        test_loss += loss.item()*data.size(0)
        # convert output probabilities to predicted class
        _, pred = torch.max(output, 1)
        # compare predictions to true label
        correct = np.squeeze(pred.eq(target.data.view_as(pred)))
        # calculate test accuracy for each object class
        for i in range(batch_size):
            label = target.data[i]
            class_correct[label] += correct[i].item()
            class_total[label] += 1

    # calculate and print avg test loss
    test_loss = test_loss/len(test_loader.dataset)
    print('Test Loss: {:.6f}\n'.format(test_loss))

    for i in range(10):
        if class_total[i] > 0:
            print('Test Accuracy of %5s: %2d%% (%2d/%2d)' % (
                str(i), 100 * class_correct[i] / class_total[i],
                np.sum(class_correct[i]), np.sum(class_total[i])))

    print('\nTest Accuracy (Overall): %2d%% (%2d/%2d)' % (
        100. * np.sum(class_correct) / np.sum(class_total),
        np.sum(class_correct), np.sum(class_total)))


def main():
    model = MLP_net()
    print(model)

if __name__ == "__main__":
    main()