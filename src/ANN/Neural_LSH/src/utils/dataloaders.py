import torch
from torch.utils.data import Dataset, random_split

# convert data and labels to dataloaders
class VectorDataset(Dataset):
    def __init__(self, X, y):
        self.X = torch.tensor(X, dtype=torch.float32)
        self.y = torch.tensor(y, dtype=torch.long)   
    def __len__(self):
        return len(self.X)
    def __getitem__(self, idx):
        return self.X[idx], self.y[idx]
    

def create_loaders(X, y, batch_size=128, valid_ratio=0.2):
    dataset = VectorDataset(X, y)

    N = len(dataset)
    n_valid = int(N * valid_ratio)
    n_train = N - n_valid

    train_set, valid_set = random_split(dataset, [n_train, n_valid])

    train_loader = torch.utils.data.DataLoader(
        train_set, batch_size=batch_size, shuffle=True, drop_last=False
    )

    valid_loader = torch.utils.data.DataLoader(
        valid_set, batch_size=batch_size, shuffle=False, drop_last=False
    )

    return train_loader, valid_loader