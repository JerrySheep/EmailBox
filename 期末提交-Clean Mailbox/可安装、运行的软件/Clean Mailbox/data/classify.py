# -*- coding: utf-8 -*-
"""
Created on Fri May 19 09:27:44 2017

@author: Jason
"""
import csv
import re
import jieba
import os

class spamEmailBayes:
    #获得停用词表
    def getStopWords(self):
        stopList=[]
        for line in open("data/中文停用词表.txt"):
            stopList.append(line[:len(line)-1])
        return stopList;
    
    #获得词典
    def get_word_list(self,content,wordsList,stopList):
        #分词结果放入res_list
        res_list = list(jieba.cut(content))
        for i in res_list:
            if i not in stopList and i.strip()!='' and i!=None:
                if i not in wordsList:
                    wordsList.append(i)
    
    #若列表中的词已在词典中，则加1，否则添加进去
    def addToDict(self,wordsList,wordsDict):
        for item in wordsList:
            if item in wordsDict.keys():
                wordsDict[item]+=1
            else:
                wordsDict.setdefault(item,1)                
    
    def get_dict(self, fileName):
        Dict = {}
        with open(fileName) as file:
            lines = csv.reader(file)
            for i,line in enumerate(lines):
                if i==0:
                    fileLen=int(line[0])
                else:
                    Dict[line[0]]=int(line[1])
        return Dict, fileLen
    
    #通过计算每个文件中p(s|w)来得到对分类影响最大的15个词
    def getTestWords(self,testDict,spamDict,normDict,normFilelen,spamFilelen):
        wordProbList={}
        for word,num  in testDict.items():
            if word in spamDict.keys() and word in normDict.keys():
                #该文件中包含词个数
                pw_s=spamDict[word]/spamFilelen
                pw_n=normDict[word]/normFilelen
                ps_w=pw_s*spamFilelen/(pw_s*spamFilelen+pw_n*normFilelen) 
                wordProbList.setdefault(word,ps_w)
            if word in spamDict.keys() and word not in normDict.keys():
                pw_s=spamDict[word]/spamFilelen
                pw_n=0.01
                ps_w=pw_s*spamFilelen/(pw_s*spamFilelen+pw_n*normFilelen) 
                wordProbList.setdefault(word,ps_w)
            if word not in spamDict.keys() and word in normDict.keys():
                pw_s=0.01
                pw_n=normDict[word]/normFilelen
                ps_w=pw_s*spamFilelen/(pw_s*spamFilelen+pw_n*normFilelen) 
                wordProbList.setdefault(word,ps_w)
            if word not in spamDict.keys() and word not in normDict.keys():
                #若该词不在两个词典中，概率设为0.4
                wordProbList.setdefault(word,0.4)
        sorted(wordProbList.items(),key=lambda d:d[1],reverse=True)[0:15]
        return (wordProbList)
    
    #计算贝叶斯概率
    def calBayes(self,wordList,spamdict,normdict):
        ps_w=1
        ps_n=1
         
        for word,prob in wordList.items() :
            ps_w*=(prob)
            ps_n*=(1-prob)
        p=ps_w/(ps_w+ps_n)
        return p

#spam类对象
spam=spamEmailBayes()

stopList=spam.getStopWords()        #获得停用词表，用于对停用词过滤
FileList=os.listdir("temp")         #获取要识别的邮件文本列表
SpamList=os.listdir("data/spam")    #获取要学习的邮件文本

normDict, normFilelen = spam.get_dict('data/ham.csv')    #从csv文件中获取正常、垃圾词频
spamDict, spamFilelen = spam.get_dict('data/spam.csv')

wordsList ,wordsDict = [], {}       #邮件的词列表和词字典
for fileName in SpamList:           #学习用户的垃圾分类
    wordsList.clear()
    for line in open("data/spam/"+fileName):
        rule=re.compile(r"[^\u4e00-\u9fa5]")
        line=rule.sub("",line)
        spam.get_word_list(line,wordsList,stopList)
    spam.addToDict(wordsList, spamDict)
spamFilelen += len(SpamList)        #更新垃圾邮件长度

results = []
for fileName in FileList:
    wordsDict.clear()
    wordsList.clear()
    for line in open("temp/"+fileName):
        rule=re.compile(r"[^\u4e00-\u9fa5]")
        line=rule.sub("",line)
        spam.get_word_list(line,wordsList,stopList)
    spam.addToDict(wordsList, wordsDict)
    #通过计算每个文件中p(s|w)来得到对分类影响最大的15个词
    wordProbList=spam.getTestWords(wordsDict, spamDict,normDict,normFilelen,spamFilelen)      
    p=spam.calBayes(wordProbList, spamDict, normDict) #对每封邮件得到的15个词计算贝叶斯概率
    results.append(1 if p>0.9 else 0)

print(results)

with open('data/ham.csv','w',newline='') as myFile:  #保存正常词频文件
    myWriter=csv.writer(myFile)
    myWriter.writerow([normFilelen])
    for (k,v) in normDict.items():
        myWriter.writerow([k,v])         
with open('data/spam.csv','w',newline='') as myFile:  #保存垃圾词频文件  
    myWriter=csv.writer(myFile)
    myWriter.writerow([spamFilelen])
    for (k,v) in spamDict.items():
        myWriter.writerow([k,v])
with open("temp/results.txt",'w') as file:            #保存结果文件
    for i in results:
        print(str(i)+" "),
        file.write(str(i)+" ")