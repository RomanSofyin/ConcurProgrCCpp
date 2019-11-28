#include <cstdlib>
#include <iostream>

#include <list>
#include <utility>
#include <cstring>

using namespace std;
/* Следующий вариант и ему подобные считаются читерством
class SmallAllocator {
public:
        void *Alloc(unsigned int Size) {
                return malloc(Size);
        };
        void *ReAlloc(void *Pointer, unsigned int Size) {
                return realloc(Pointer, Size);
        };
        void Free(void *Pointer) {
                return free(Pointer);
        };
};*/

class SmallAllocator {
private:
        char Memory[1048576];
		using ChunkLengthType = size_t;
		static const size_t ctrlDataLen = sizeof(ChunkLengthType);
        using ChunkDescr = pair<size_t, ChunkLengthType>;	// индекс начала блока в Memory и его длина
		list<ChunkDescr> freeChunks;	// вектор свободных непрерывных блоков памяти
public:
		SmallAllocator() : freeChunks() {
			cout << "SmallAllocator(): gets called" << endl;
			ChunkDescr p = make_pair(0, sizeof(Memory));
			freeChunks.push_back(p);
			cout << "SmallAllocator(): finishing" << endl;
		}

		// каждый выделенный блок памяти в начале будет содержать свою длину
        void *Alloc(unsigned int Size) {
			cout << "Alloc(): " << Size << " byte(s) requested" << endl;
        	// ищём первый свободдный блок памяти достаточного размера
        	//	для этого обходим freeChunks, проверяя длину каждого
        	// возвращаем найденный блок
			size_t occupiedChunkLen = Size + ctrlDataLen;
			//for(const ChunkDescr& cd : freeChunks) {
			for(list<ChunkDescr>::iterator it = freeChunks.begin(); it != freeChunks.end(); it++) {	// it++ or ++it ? ? ? ? ?
				if (it->second >= occupiedChunkLen) {
					unsigned int * p = (unsigned int *) &Memory[it->first];
					*p = Size;
					void * res = (void *) &Memory[it->first + ctrlDataLen];
					// проверить случай, когда найденный свободный блок "идеально" подходит по длине под размер запрашиваемого блока
					// в этом случае не нужно создавать новый ChunkDescr
					if (it->second > occupiedChunkLen) {
						auto arg1 = it->first + occupiedChunkLen;
						auto arg2 = it->second - occupiedChunkLen;
						/* Why this doesn't work ? ? ? 
						cout << "Alloc(): constructing new chunk" << endl;
						freeChunks.emplace(it, it->first + occupiedChunkLen, it->second - occupiedChunkLen);
						it++;
						cout << "Alloc(): erasing old chunk" << endl;						
						freeChunks.erase(it);*/
						cout << "Alloc(): erasing old chunk" << endl;						
						freeChunks.erase(it);
						cout << "Alloc(): constructing new chunk" << endl;
						freeChunks.emplace(it, arg1, arg2);
					}
					cout << "Alloc(): returning res" << endl;
					return res;
				}
			}
			cout << "Alloc(): returning NULL" << endl;
        	return NULL;
        }
        
		void *ReAlloc(void *Pointer, unsigned int Size) {
			cout << "ReAlloc(): reallocate " << Size << " byte(s)" << endl;
			void * res = Alloc(Size);
			memcpy(res, Pointer,  static_cast<size_t>(*(static_cast<char*>(Pointer)-ctrlDataLen)));
			Free(Pointer);
			cout << "ReAlloc(): returning res" << endl;
			return res;
		}
        
		void Free(void *Pointer) {
			size_t memIndex = (static_cast<char*>(Pointer) - ctrlDataLen) - &Memory[0];
			for(auto it = freeChunks.begin(); it != freeChunks.end(); it++) {
				if(memIndex = it->first) {
					freeChunks.erase(it);
				}
			}
			
			return;
		}
};

int main(int argc, char **argv) {
	// Программа не должна кориться и не должна выводить текст.
	SmallAllocator A1;
	int * A1_P1 = (int *) A1.Alloc(sizeof(int));
	A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
	A1.Free(A1_P1);
	
	SmallAllocator A2;
	int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
	// инициализация памяти
	for(unsigned int i = 0; i < 10; i++)
		A2_P1[i] = i;
	// проверки содержимого памяти
	for(unsigned int i = 0; i < 10; i++)
		if(A2_P1[i] != i)
			std::cout << "ERROR 1" << std::endl;
	
	int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
	// инициализация памяти
	for(unsigned int i = 0; i < 10; i++) 
		A2_P2[i] = -1;
	// проверки содержимого памяти
	for(unsigned int i = 0; i < 10; i++) 
		if(A2_P1[i] != i) 
			std::cout << "ERROR 2" << std::endl;
	for(unsigned int i = 0; i < 10; i++) 
		if(A2_P2[i] != -1) 
			std::cout << "ERROR 3" << std::endl;
	
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
	// инициализация памяти
	for(unsigned int i = 10; i < 20; i++) 
		A2_P1[i] = i;
	// проверки содержимого памяти
	for(unsigned int i = 0; i < 20; i++) 
		if(A2_P1[i] != i) 
			std::cout << "ERROR 4" << std::endl;
	for(unsigned int i = 0; i < 10; i++) 
		if(A2_P2[i] != -1) 
			std::cout << "ERROR 5" << std::endl;
	
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
	// проверки содержимого памяти
	for(unsigned int i = 0; i < 5; i++) 
		if(A2_P1[i] != i) 
			std::cout << "ERROR 6" << std::endl;
	for(unsigned int i = 0; i < 10; i++) 
		if(A2_P2[i] != -1) 
			std::cout << "ERROR 7" << std::endl;
	
	A2.Free(A2_P1);
	A2.Free(A2_P2);
	
	return 0;
}
