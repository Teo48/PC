#include <bits/stdc++.h>

bool isBST(int arr[], int n, int &depth){

int root = INT_MAX;

std::stack<int>s;

for (int i = n - 1; i >= 0; --i) {
    if (arr[i] > root) {
        return false;
    }

    while (!s.empty() && arr[i] < s.top()) {
        root = s.top();
        s.pop();
        ++depth;
    }

    s.push(arr[i]);
  }
  return true;
}

int main() {
    int arr[] = {1, 2, 4, 5, 3};
    int depth = 0;
    bool is_valid = isBST(arr, 5, depth);

    if (is_valid) {
        std::cout << depth;
    } else {
        std::cout << "-1\n"; 
    }

    return 0;
}
