/*Huffman tree node*/
struct Node {
    float c_location;        // c_location
    int freq;       // freq
    struct Node *left, *right;  // left * right pointer
};

// Huffman: make new node
struct Node* newNode(float c_location, int freq) {
    struct Node* node = (struct Node*) malloc(sizeof(struct Node));
    node->c_location = c_location;
    node->freq = freq;
    node->left = node->right = NULL;
    return node;
}

// compacte two two node 
struct Node* mergeNodes(struct Node* left, struct Node* right) {
    struct Node* node = newNode(NULL, left->freq + right->freq);
    node->left = left;
    node->right = right;
    return node;
}

// Huffman 트리 생성 함수
struct Node* buildHuffmanTree(char* input, int* freq) {
    int n = strlen(input);

    // 빈도수 정보로부터 leaf node 생성
    struct Node** nodes = (struct Node**) malloc(n * sizeof(struct Node*));
    for (int i = 0; i < n; i++) {
        nodes[i] = newNode(input[i], freq[i]);
    }

    // 노드들을 정렬하고 Huffman 트리 생성
    while (n > 1) {
        qsort(nodes, n, sizeof(struct Node*), compareNodes);
        struct Node* left = nodes[n - 1];
        struct Node* right = nodes[n - 2];
        struct Node* parent = mergeNodes(left, right);
        nodes[n - 2] = parent;
        n--;
    }

    // 마지막에 남은 루트 노드를 반환
    return nodes[0];
}

// Huffman 코드 생성 함수
void generateHuffmanCodes(struct Node* node, char* code, int depth) {
    if (node == NULL) {
        return;
    }

    if (node->left == NULL && node->right == NULL) {
        printf("%c: %s\n", node->ch, code);
    }