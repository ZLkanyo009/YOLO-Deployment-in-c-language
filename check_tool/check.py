out = 1

if out == 1:
    f1 = open("./out_face/out10", "r")
    
    #f2 = open("/home/zldlg/yolov2-yolov3_PyTorch/log_file/out_face/out10", "r")
    f2 = open("/home/zldlg/yolov2-yolov3_PyTorch/log_file/copy/out10", "r")
else:
    f1 = open("test", "r")
    #f2 = open("/home/zldlg/yolov2-yolov3_PyTorch/log_file/inp_face/in6", "r")
    f2 = open("/home/zldlg/yolov2-yolov3_PyTorch/test", "r")

lines1 = f1.readlines()
lines2 = f2.readlines()
for index, line1 in enumerate(lines1):
    list1 = lines1[index].strip("\n").split("\t")
    list2 = lines2[index].strip("\n").split("\t")
    if "==" in list1[0] and "==" in list2[0]:
        print(list1[0])
    # print("=============1===================")
    # print(list1)
    # print("=============2===================")
    # print(list2)
    for i in range(0, len(list1)):
        try :
            print(int(list1[i]) - int(list2[i]), end = "\t")
        except :
            print("!!!!!!!!!!!!!!", end="")
            print(list1[i], " " ,list2[i])
    print("")