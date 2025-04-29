#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md51.h"
#include <iomanip>
using namespace std;
using namespace chrono;
// 编译指令如下
// g++ main1.cpp train.cpp guessing.cpp md51.cpp -o main
// g++ main1.cpp train.cpp guessing.cpp md51.cpp -o main -O1
// g++ main1.cpp train.cpp guessing.cpp md51.cpp -o main -O2
int main() {
    double time_hash = 0;
    double time_guess = 0;
    double time_train = 0;
    PriorityQueue q;
    
    auto start_train = system_clock::now();
    q.m.train("/guessdata/Rockyou-singleLined-full.txt");
    q.m.order();
    auto end_train = system_clock::now();
    auto duration_train = duration_cast<microseconds>(end_train - start_train);
    time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;

    q.init();
    cout << "here" << endl;
    int curr_num = 0;
    auto start = system_clock::now();
    int history = 0;
    
    while (!q.priority.empty()) {
        q.PopNext();
        q.total_guesses = q.guesses.size();
        
        if (q.total_guesses - curr_num >= 100000) {
            cout << "Guesses generated: " << history + q.total_guesses << endl;
            curr_num = q.total_guesses;

            int generate_n = 10000000;
            if (history + q.total_guesses > generate_n) {
                auto end = system_clock::now();
                auto duration = duration_cast<microseconds>(end - start);
                time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << " seconds" << endl;
                cout << "Hash time:" << time_hash << " seconds" << endl;
                cout << "Train time:" << time_train << " seconds" << endl;
                break;
            }
        }
        
        if (curr_num > 1000000) {
            auto start_hash = system_clock::now();
            bit32 states[4][4]; // 存储四个MD5结果
            
            // 每次处理4个密码
            for (int i = 0; i < q.guesses.size(); i += 4) {
                string inputs[4];
                for (int j = 0; j < 4 && (i + j) < q.guesses.size(); j++) {
                    inputs[j] = q.guesses[i + j];
                }
                
                // 使用NEON优化的MD5计算
                MD5Hash_NEON(inputs, states);
                
                // 输出结果（可选）
                /*
                for (int j = 0; j < 4 && (i + j) < q.guesses.size(); j++) {
                    cout << inputs[j] << "\t";
                    for (int k = 0; k < 4; k++) {
                        cout << hex << setw(8) << setfill('0') << states[k][j];
                    }
                    cout << endl;
                }
                */
            }
            
            auto end_hash = system_clock::now();
            auto duration = duration_cast<microseconds>(end_hash - start_hash);
            time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;

            history += curr_num;
            curr_num = 0;
            q.guesses.clear();
        }
    }
    return 0;
}
