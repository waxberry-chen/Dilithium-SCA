import linecache
### get distance in cycle 7

a_last = 0
qNUM = 8380417

########################################
#     Get Prediction Vector Method     #
########################################
def distance(plaintexts,key):
    global a_last
    p0 = 0
    for i in range(3):
        p0 += HD((plaintexts[2-i]*key),(plaintexts[3-i]*key))

    input_7 = HD(plaintexts[5], 0)
    input_6 = HD(plaintexts[4], plaintexts[5])
    input_5 = HD(plaintexts[3], plaintexts[4])
    input_4 = HD(plaintexts[2], plaintexts[3])
    input_3 = HD(plaintexts[1], plaintexts[2])
    input_2 = HD(plaintexts[0], plaintexts[1])
    input_1 = HD(a_last, plaintexts[0])

    mm_result = HD(plaintexts[0]*key%qNUM, plaintexts[1]*key%qNUM)
    output = HD((a_last*key)%qNUM, (plaintexts[0]*key)%qNUM)

    a_last = plaintexts[4]
    # return output + mm_result
    return  mm_result+output

    # ##### Verify #####
    # wn = 1729
    # N = 3329
    # a_val = plaintexts[0]
    # product_guess = (key * wn) % N
    # c_sum = a_val + product_guess
    # c_val = c_sum - N if c_sum >= N else c_sum
    # return HD(c_val, 0)


def HD(num1,num2):
    return bin(num2^num1).count('1')

def get_plaintexts(file_path,trace_number,plaintext_num=6):
    plaintexts = []
    for i in range(plaintext_num):
        line = linecache.getline(file_path, trace_number+i+1).rstrip('\n') #从1开始(HERE)
        if not line or line.isspace():
            raise ValueError(f"Plaintexts file line num not enough or cant find file {file_path}")
        plaintexts.append(int(line)) 
    return plaintexts 