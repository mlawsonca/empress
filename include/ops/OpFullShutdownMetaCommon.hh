/*
 * Copyright 2018 National Technology & Engineering Solutions of
 * Sandia, LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS,
 * the U.S. Government retains certain rights in this software.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Sandia Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef OPFULLSHUTDOWNMETACOMMON_HH
#define OPFULLSHUTDOWNMETACOMMON_HH

#include "opbox/OpBox.hh"
#include "lunasa/DataObject.hh"

extern bool debug;

class OpFullShutdownMeta : public opbox::Op {

 enum class State : int  {
      start=0,
      done };

public:
  OpFullShutdownMeta(net::peer_ptr_t peer);
  OpFullShutdownMeta(op_create_as_target_t t);
  ~OpFullShutdownMeta();

  //Unique name and id for this op
  const static unsigned int op_id;
  const static std::string  op_name;  

  unsigned int getOpID() const { return op_id; }
  std::string  getOpName() const { return op_name; }


  WaitingType UpdateOrigin(OpArgs &args, results_t *results);
  WaitingType UpdateTarget(OpArgs &args, results_t *results);
 
  std::string GetStateName() const;


private:
  State state;
  net::peer_t *peer;

  //void log(const std::string &s);

  
  void createOutgoingMessage(gutties::nodeid_t dst, 
                                   const mailbox_t &src_mailbox, 
                                   const mailbox_t &dst_mailbox);

  lunasa::DataObject *ldo_msg;

};


#endif // OPFULLSHUTDOWNMETACOMMON_HH
