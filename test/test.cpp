#include <include/db.hpp>

char *random_str(unsigned int size) {
    char *str = (char *)malloc(size + 1);
    for (unsigned int i = 0; i < size; i++) {
        switch (rand() % 3) {
        case 0:
            str[i] = rand() % 10 + '0';
            break;
        case 1:
            str[i] = rand() % 26 + 'A';
            break;
        case 2:
            str[i] = rand() % 26 + 'a';
            break;
        default:
            break;
        }
    }
    str[size] = 0;

    return str;
}

bool assert_is_same(const char * r , const char * w){
    return strcmp(r , w) == 0;
}

int main() {
    DB *db = nullptr;
    DB::CreateOrOpen("./tmp", &db);
    Slice k;
    k.size() = 16;
    Slice v;
    v.size() = 80;
    int times = 100;
    printf("[start]...\n");
    while (times--) {
        k.data() = random_str(16);
        v.data() = random_str(80);
        db->Set(k, v);
        std::string a;
        auto sta = db->Get(k, &a);

        if (sta != Status::Ok || !assert_is_same(a.data() , v.data())){
            printf("[error]:read {%s} , write {%s} \n" , a.data() , v.data());
        }

        free(k.data());
        free(v.data());
    }

    printf("[finish]\n");
    return 0;
}