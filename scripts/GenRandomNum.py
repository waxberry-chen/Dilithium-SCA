import random

file_root_path = '/15T/Projects/Dilithium-SCA/data/special_files/' 




def gen_random(file,number,num_arange=8380417,num_start=1):
    with open(file,'w') as of:
        unique_randoms = random.sample(range(num_start, num_arange), k=number)
        #unique_randoms = range(3329+6)
        for index,num in enumerate(unique_randoms):
            of.write(str(num)+'\n')
            # if index %2 == 0:
            #     of.write(str(num)+'\n')
            # else :
            #     of.write(str(251210)+'\n')
    print(f"--->Gen {number} num in [{num_start},{num_arange-1}], Saved in file: {file}")

# def gen_power_test(file,number,num_arange=8380417,num_start=1):
#     with open(file,'w') as of:
#         #unique_randoms = random.sample(range(num_start, num_arange), k=number)
#         unique_randoms = range(3329+6)
#         value_num = 0
#         for num in unique_randoms:
#             if num %2 ==0:
                
#                 if num%6 == 0:
#                     value_num +=1
#                 of.write(str(2**(value_num%24)-1)+'\n')
#             else:
#                 of.write(str(0)+'\n')
#     print(f"--->Gen {number} num in [{num_start},{num_arange-1}], Saved in file: {file}")

if __name__ == "__main__":
    number = 50006
    file_name = f'Random_{number}_dil.txt'
    file = file_root_path + file_name
    gen_random(file=file, number=number)
    # file_name = f'power_test_{number}.txt'
    # file = file_root_path + file_name
    # gen_power_test(file=file, number=number)