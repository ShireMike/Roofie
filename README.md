# Roofie
# this program requires file /etc/roofcheck.conf as follows:
#
#################################
#  Email info
#  Max of 5 entries each 
#  for smptto, smptcc and smptbcc
#################################
#
smtpfrom   Bong Bong Observatory <noreply@bongbong.net>    
smtpto     astronomaer fred <fred@bongbong.net>
smtpto     astronomaer bill <bill@bongbong.net>
smtpcc     admin <admin@bongbong.net>    
smtpbcc    IT Support <suppport@bongbong.net> 
address    Bong Bong, Bongo Bongo Land
#
##################################
#  FTP server info
##################################
ftpuser  bong1234
ftppass  password
ftpurl   ftp.bongbong.net/public_html/test/
#
# FTP client files to upload (max 10)
#
ftpdir  /var/gco/
ftpfile roofstatus.html
ftpfile list.html
ftpfile title.html
ftpfile rooflog.html
################################
