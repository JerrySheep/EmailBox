# -*- coding: utf-8 -*-
"""
Created on Fri May 19 09:27:44 2017

@author: Jason
"""
import csv
import re
import jieba

class spamEmailBayes:
    #获得停用词表
    def getStopWords(self):
        stopList=[]
        for line in open("F:/Clean_MailBox/中文停用词表.txt"):
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
            for line in lines:
                Dict[line[0]]=int(line[1])
        return Dict
    
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
        print(ps_w, ps_n, p)
        return p

#spam类对象
spam=spamEmailBayes()
#获得停用词表，用于对停用词过滤
stopList=spam.getStopWords()

#保存邮件出现的的词
wordsList ,wordsDict = [], {}

#从csv文件中获取正常词频
normDict = spam.get_dict('F:/Clean_MailBox/ham.csv')
#从csv文件中获取垃圾邮件次词频
spamDict = spam.get_dict('F:/Clean_MailBox/spam.csv')
normFilelen, spamFilelen = 7063, 7775

#email = input("请输入邮件：")
email = "本公司长期经营外包项目，有意者请拨打12345679"
rule=re.compile(r"[^\u4e00-\u9fa5]")
line=rule.sub("",email)
spam.get_word_list(line,wordsList,stopList)

spam.addToDict(wordsList, wordsDict)
testDict=wordsDict.copy()
#通过计算每个文件中p(s|w)来得到对分类影响最大的15个词
wordProbList=spam.getTestWords(testDict, spamDict,normDict,normFilelen,spamFilelen)
#对每封邮件得到的15个词计算贝叶斯概率  
p=spam.calBayes(wordProbList, spamDict, normDict)
result = 1 if p>0.9 else 0