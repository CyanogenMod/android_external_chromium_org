// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is auto-generated from
// gpu/command_buffer/build_gles2_cmd_buffer.py
// DO NOT EDIT!

#ifndef GPU_COMMAND_BUFFER_COMMON_GLES2_CMD_IDS_AUTOGEN_H_
#define GPU_COMMAND_BUFFER_COMMON_GLES2_CMD_IDS_AUTOGEN_H_

#define GLES2_COMMAND_LIST(OP) \
  OP(ActiveTexture)                                            /* 256 */ \
  OP(AttachShader)                                             /* 257 */ \
  OP(BindAttribLocation)                                       /* 258 */ \
  OP(BindAttribLocationImmediate)                              /* 259 */ \
  OP(BindAttribLocationBucket)                                 /* 260 */ \
  OP(BindBuffer)                                               /* 261 */ \
  OP(BindFramebuffer)                                          /* 262 */ \
  OP(BindRenderbuffer)                                         /* 263 */ \
  OP(BindTexture)                                              /* 264 */ \
  OP(BlendColor)                                               /* 265 */ \
  OP(BlendEquation)                                            /* 266 */ \
  OP(BlendEquationSeparate)                                    /* 267 */ \
  OP(BlendFunc)                                                /* 268 */ \
  OP(BlendFuncSeparate)                                        /* 269 */ \
  OP(BufferData)                                               /* 270 */ \
  OP(BufferDataImmediate)                                      /* 271 */ \
  OP(BufferSubData)                                            /* 272 */ \
  OP(BufferSubDataImmediate)                                   /* 273 */ \
  OP(CheckFramebufferStatus)                                   /* 274 */ \
  OP(Clear)                                                    /* 275 */ \
  OP(ClearColor)                                               /* 276 */ \
  OP(ClearDepthf)                                              /* 277 */ \
  OP(ClearStencil)                                             /* 278 */ \
  OP(ColorMask)                                                /* 279 */ \
  OP(CompileShader)                                            /* 280 */ \
  OP(CompressedTexImage2D)                                     /* 281 */ \
  OP(CompressedTexImage2DImmediate)                            /* 282 */ \
  OP(CompressedTexImage2DBucket)                               /* 283 */ \
  OP(CompressedTexSubImage2D)                                  /* 284 */ \
  OP(CompressedTexSubImage2DImmediate)                         /* 285 */ \
  OP(CompressedTexSubImage2DBucket)                            /* 286 */ \
  OP(CopyTexImage2D)                                           /* 287 */ \
  OP(CopyTexSubImage2D)                                        /* 288 */ \
  OP(CreateProgram)                                            /* 289 */ \
  OP(CreateShader)                                             /* 290 */ \
  OP(CullFace)                                                 /* 291 */ \
  OP(DeleteBuffers)                                            /* 292 */ \
  OP(DeleteBuffersImmediate)                                   /* 293 */ \
  OP(DeleteFramebuffers)                                       /* 294 */ \
  OP(DeleteFramebuffersImmediate)                              /* 295 */ \
  OP(DeleteProgram)                                            /* 296 */ \
  OP(DeleteRenderbuffers)                                      /* 297 */ \
  OP(DeleteRenderbuffersImmediate)                             /* 298 */ \
  OP(DeleteShader)                                             /* 299 */ \
  OP(DeleteTextures)                                           /* 300 */ \
  OP(DeleteTexturesImmediate)                                  /* 301 */ \
  OP(DepthFunc)                                                /* 302 */ \
  OP(DepthMask)                                                /* 303 */ \
  OP(DepthRangef)                                              /* 304 */ \
  OP(DetachShader)                                             /* 305 */ \
  OP(Disable)                                                  /* 306 */ \
  OP(DisableVertexAttribArray)                                 /* 307 */ \
  OP(DrawArrays)                                               /* 308 */ \
  OP(DrawElements)                                             /* 309 */ \
  OP(Enable)                                                   /* 310 */ \
  OP(EnableVertexAttribArray)                                  /* 311 */ \
  OP(Finish)                                                   /* 312 */ \
  OP(Flush)                                                    /* 313 */ \
  OP(FramebufferRenderbuffer)                                  /* 314 */ \
  OP(FramebufferTexture2D)                                     /* 315 */ \
  OP(FrontFace)                                                /* 316 */ \
  OP(GenBuffers)                                               /* 317 */ \
  OP(GenBuffersImmediate)                                      /* 318 */ \
  OP(GenerateMipmap)                                           /* 319 */ \
  OP(GenFramebuffers)                                          /* 320 */ \
  OP(GenFramebuffersImmediate)                                 /* 321 */ \
  OP(GenRenderbuffers)                                         /* 322 */ \
  OP(GenRenderbuffersImmediate)                                /* 323 */ \
  OP(GenTextures)                                              /* 324 */ \
  OP(GenTexturesImmediate)                                     /* 325 */ \
  OP(GetActiveAttrib)                                          /* 326 */ \
  OP(GetActiveUniform)                                         /* 327 */ \
  OP(GetAttachedShaders)                                       /* 328 */ \
  OP(GetAttribLocation)                                        /* 329 */ \
  OP(GetAttribLocationImmediate)                               /* 330 */ \
  OP(GetAttribLocationBucket)                                  /* 331 */ \
  OP(GetBooleanv)                                              /* 332 */ \
  OP(GetBufferParameteriv)                                     /* 333 */ \
  OP(GetError)                                                 /* 334 */ \
  OP(GetFloatv)                                                /* 335 */ \
  OP(GetFramebufferAttachmentParameteriv)                      /* 336 */ \
  OP(GetIntegerv)                                              /* 337 */ \
  OP(GetProgramiv)                                             /* 338 */ \
  OP(GetProgramInfoLog)                                        /* 339 */ \
  OP(GetRenderbufferParameteriv)                               /* 340 */ \
  OP(GetShaderiv)                                              /* 341 */ \
  OP(GetShaderInfoLog)                                         /* 342 */ \
  OP(GetShaderPrecisionFormat)                                 /* 343 */ \
  OP(GetShaderSource)                                          /* 344 */ \
  OP(GetString)                                                /* 345 */ \
  OP(GetTexParameterfv)                                        /* 346 */ \
  OP(GetTexParameteriv)                                        /* 347 */ \
  OP(GetUniformfv)                                             /* 348 */ \
  OP(GetUniformiv)                                             /* 349 */ \
  OP(GetUniformLocation)                                       /* 350 */ \
  OP(GetUniformLocationImmediate)                              /* 351 */ \
  OP(GetUniformLocationBucket)                                 /* 352 */ \
  OP(GetVertexAttribfv)                                        /* 353 */ \
  OP(GetVertexAttribiv)                                        /* 354 */ \
  OP(GetVertexAttribPointerv)                                  /* 355 */ \
  OP(Hint)                                                     /* 356 */ \
  OP(IsBuffer)                                                 /* 357 */ \
  OP(IsEnabled)                                                /* 358 */ \
  OP(IsFramebuffer)                                            /* 359 */ \
  OP(IsProgram)                                                /* 360 */ \
  OP(IsRenderbuffer)                                           /* 361 */ \
  OP(IsShader)                                                 /* 362 */ \
  OP(IsTexture)                                                /* 363 */ \
  OP(LineWidth)                                                /* 364 */ \
  OP(LinkProgram)                                              /* 365 */ \
  OP(PixelStorei)                                              /* 366 */ \
  OP(PolygonOffset)                                            /* 367 */ \
  OP(ReadPixels)                                               /* 368 */ \
  OP(ReleaseShaderCompiler)                                    /* 369 */ \
  OP(RenderbufferStorage)                                      /* 370 */ \
  OP(SampleCoverage)                                           /* 371 */ \
  OP(Scissor)                                                  /* 372 */ \
  OP(ShaderBinary)                                             /* 373 */ \
  OP(ShaderSource)                                             /* 374 */ \
  OP(ShaderSourceImmediate)                                    /* 375 */ \
  OP(ShaderSourceBucket)                                       /* 376 */ \
  OP(StencilFunc)                                              /* 377 */ \
  OP(StencilFuncSeparate)                                      /* 378 */ \
  OP(StencilMask)                                              /* 379 */ \
  OP(StencilMaskSeparate)                                      /* 380 */ \
  OP(StencilOp)                                                /* 381 */ \
  OP(StencilOpSeparate)                                        /* 382 */ \
  OP(TexImage2D)                                               /* 383 */ \
  OP(TexImage2DImmediate)                                      /* 384 */ \
  OP(TexParameterf)                                            /* 385 */ \
  OP(TexParameterfv)                                           /* 386 */ \
  OP(TexParameterfvImmediate)                                  /* 387 */ \
  OP(TexParameteri)                                            /* 388 */ \
  OP(TexParameteriv)                                           /* 389 */ \
  OP(TexParameterivImmediate)                                  /* 390 */ \
  OP(TexSubImage2D)                                            /* 391 */ \
  OP(TexSubImage2DImmediate)                                   /* 392 */ \
  OP(Uniform1f)                                                /* 393 */ \
  OP(Uniform1fv)                                               /* 394 */ \
  OP(Uniform1fvImmediate)                                      /* 395 */ \
  OP(Uniform1i)                                                /* 396 */ \
  OP(Uniform1iv)                                               /* 397 */ \
  OP(Uniform1ivImmediate)                                      /* 398 */ \
  OP(Uniform2f)                                                /* 399 */ \
  OP(Uniform2fv)                                               /* 400 */ \
  OP(Uniform2fvImmediate)                                      /* 401 */ \
  OP(Uniform2i)                                                /* 402 */ \
  OP(Uniform2iv)                                               /* 403 */ \
  OP(Uniform2ivImmediate)                                      /* 404 */ \
  OP(Uniform3f)                                                /* 405 */ \
  OP(Uniform3fv)                                               /* 406 */ \
  OP(Uniform3fvImmediate)                                      /* 407 */ \
  OP(Uniform3i)                                                /* 408 */ \
  OP(Uniform3iv)                                               /* 409 */ \
  OP(Uniform3ivImmediate)                                      /* 410 */ \
  OP(Uniform4f)                                                /* 411 */ \
  OP(Uniform4fv)                                               /* 412 */ \
  OP(Uniform4fvImmediate)                                      /* 413 */ \
  OP(Uniform4i)                                                /* 414 */ \
  OP(Uniform4iv)                                               /* 415 */ \
  OP(Uniform4ivImmediate)                                      /* 416 */ \
  OP(UniformMatrix2fv)                                         /* 417 */ \
  OP(UniformMatrix2fvImmediate)                                /* 418 */ \
  OP(UniformMatrix3fv)                                         /* 419 */ \
  OP(UniformMatrix3fvImmediate)                                /* 420 */ \
  OP(UniformMatrix4fv)                                         /* 421 */ \
  OP(UniformMatrix4fvImmediate)                                /* 422 */ \
  OP(UseProgram)                                               /* 423 */ \
  OP(ValidateProgram)                                          /* 424 */ \
  OP(VertexAttrib1f)                                           /* 425 */ \
  OP(VertexAttrib1fv)                                          /* 426 */ \
  OP(VertexAttrib1fvImmediate)                                 /* 427 */ \
  OP(VertexAttrib2f)                                           /* 428 */ \
  OP(VertexAttrib2fv)                                          /* 429 */ \
  OP(VertexAttrib2fvImmediate)                                 /* 430 */ \
  OP(VertexAttrib3f)                                           /* 431 */ \
  OP(VertexAttrib3fv)                                          /* 432 */ \
  OP(VertexAttrib3fvImmediate)                                 /* 433 */ \
  OP(VertexAttrib4f)                                           /* 434 */ \
  OP(VertexAttrib4fv)                                          /* 435 */ \
  OP(VertexAttrib4fvImmediate)                                 /* 436 */ \
  OP(VertexAttribPointer)                                      /* 437 */ \
  OP(Viewport)                                                 /* 438 */ \
  OP(BlitFramebufferEXT)                                       /* 439 */ \
  OP(RenderbufferStorageMultisampleEXT)                        /* 440 */ \
  OP(TexStorage2DEXT)                                          /* 441 */ \
  OP(GenQueriesEXT)                                            /* 442 */ \
  OP(GenQueriesEXTImmediate)                                   /* 443 */ \
  OP(DeleteQueriesEXT)                                         /* 444 */ \
  OP(DeleteQueriesEXTImmediate)                                /* 445 */ \
  OP(BeginQueryEXT)                                            /* 446 */ \
  OP(EndQueryEXT)                                              /* 447 */ \
  OP(InsertEventMarkerEXT)                                     /* 448 */ \
  OP(PushGroupMarkerEXT)                                       /* 449 */ \
  OP(PopGroupMarkerEXT)                                        /* 450 */ \
  OP(GenVertexArraysOES)                                       /* 451 */ \
  OP(GenVertexArraysOESImmediate)                              /* 452 */ \
  OP(DeleteVertexArraysOES)                                    /* 453 */ \
  OP(DeleteVertexArraysOESImmediate)                           /* 454 */ \
  OP(IsVertexArrayOES)                                         /* 455 */ \
  OP(BindVertexArrayOES)                                       /* 456 */ \
  OP(SwapBuffers)                                              /* 457 */ \
  OP(GetMaxValueInBufferCHROMIUM)                              /* 458 */ \
  OP(GenSharedIdsCHROMIUM)                                     /* 459 */ \
  OP(DeleteSharedIdsCHROMIUM)                                  /* 460 */ \
  OP(RegisterSharedIdsCHROMIUM)                                /* 461 */ \
  OP(EnableFeatureCHROMIUM)                                    /* 462 */ \
  OP(ResizeCHROMIUM)                                           /* 463 */ \
  OP(GetRequestableExtensionsCHROMIUM)                         /* 464 */ \
  OP(RequestExtensionCHROMIUM)                                 /* 465 */ \
  OP(GetMultipleIntegervCHROMIUM)                              /* 466 */ \
  OP(GetProgramInfoCHROMIUM)                                   /* 467 */ \
  OP(CreateStreamTextureCHROMIUM)                              /* 468 */ \
  OP(DestroyStreamTextureCHROMIUM)                             /* 469 */ \
  OP(GetTranslatedShaderSourceANGLE)                           /* 470 */ \
  OP(PostSubBufferCHROMIUM)                                    /* 471 */ \
  OP(TexImageIOSurface2DCHROMIUM)                              /* 472 */ \
  OP(CopyTextureCHROMIUM)                                      /* 473 */ \
  OP(DrawArraysInstancedANGLE)                                 /* 474 */ \
  OP(DrawElementsInstancedANGLE)                               /* 475 */ \
  OP(VertexAttribDivisorANGLE)                                 /* 476 */ \
  OP(GenMailboxCHROMIUM)                                       /* 477 */ \
  OP(ProduceTextureCHROMIUM)                                   /* 478 */ \
  OP(ProduceTextureCHROMIUMImmediate)                          /* 479 */ \
  OP(ConsumeTextureCHROMIUM)                                   /* 480 */ \
  OP(ConsumeTextureCHROMIUMImmediate)                          /* 481 */ \
  OP(BindUniformLocationCHROMIUM)                              /* 482 */ \
  OP(BindUniformLocationCHROMIUMImmediate)                     /* 483 */ \
  OP(BindUniformLocationCHROMIUMBucket)                        /* 484 */ \
  OP(BindTexImage2DCHROMIUM)                                   /* 485 */ \
  OP(ReleaseTexImage2DCHROMIUM)                                /* 486 */ \

enum CommandId {
  kStartPoint = cmd::kLastCommonId,  // All GLES2 commands start after this.
#define GLES2_CMD_OP(name) k ## name,
  GLES2_COMMAND_LIST(GLES2_CMD_OP)
#undef GLES2_CMD_OP
  kNumCommands
};

#endif  // GPU_COMMAND_BUFFER_COMMON_GLES2_CMD_IDS_AUTOGEN_H_

